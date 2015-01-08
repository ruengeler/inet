//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/apsk/layered/APSKLayeredReceiver.h"
#include "inet/physicallayer/contract/layered/ISymbol.h"
#include "inet/physicallayer/common/layered/LayeredReceptionDecision.h"
#include "inet/physicallayer/common/layered/LayeredReception.h"
#include "inet/physicallayer/common/layered/SignalSymbolModel.h"
#include "inet/physicallayer/common/layered/SignalSampleModel.h"
#include "inet/physicallayer/common/layered/SignalBitModel.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"
#include "inet/physicallayer/common/BandListening.h"
#include "inet/physicallayer/apsk/layered/APSKDecoder.h"
#include "inet/physicallayer/apsk/layered/APSKDemodulator.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/base/NarrowbandNoiseBase.h"
#include "inet/physicallayer/common/ListeningDecision.h"
#include "inet/physicallayer/analogmodel/ScalarAnalogModel.h"
#include "inet/physicallayer/apsk/layered/APSKRadioFrame_m.h"
#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211Serializer.h"
#include "inet/common/serializer/headerserializers/EthernetCRC.h"

namespace inet {

namespace physicallayer {

using namespace inet::serializer;

Define_Module(APSKLayeredReceiver);

APSKLayeredReceiver::APSKLayeredReceiver() :
    levelOfDetail((LevelOfDetail)-1),
    errorModel(nullptr),
    decoder(nullptr),
    demodulator(nullptr),
    pulseFilter(nullptr),
    analogDigitalConverter(nullptr),
    energyDetection(W(NaN)),
    sensitivity(W(NaN)),
    carrierFrequency(Hz(NaN)),
    bandwidth(Hz(NaN)),
    snirThreshold(NaN)
{
}

void APSKLayeredReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        errorModel = dynamic_cast<ILayeredErrorModel *>(getSubmodule("errorModel"));
        decoder = dynamic_cast<IDecoder *>(getSubmodule("decoder"));
        demodulator = dynamic_cast<IDemodulator *>(getSubmodule("demodulator"));
        pulseFilter = dynamic_cast<IPulseFilter *>(getSubmodule("pulseFilter"));
        analogDigitalConverter = dynamic_cast<IAnalogDigitalConverter *>(getSubmodule("analogDigitalConverter"));
        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        // TODO: temporary parameters
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
        snirThreshold = math::dB2fraction(par("snirThreshold"));
        const char *levelOfDetailStr = par("levelOfDetail").stringValue();
        if (strcmp("bit", levelOfDetailStr) == 0)
            levelOfDetail = BIT_DOMAIN;
        else if (strcmp("symbol", levelOfDetailStr) == 0)
            levelOfDetail = SYMBOL_DOMAIN;
        else if (strcmp("sample", levelOfDetailStr) == 0)
            levelOfDetail = SAMPLE_DOMAIN;
        else
            throw cRuntimeError("Unknown level of detail='%s'", levelOfDetailStr);
        if (levelOfDetail >= BIT_DOMAIN && !decoder)
            throw cRuntimeError("Decoder not configured");
        if (levelOfDetail >= SYMBOL_DOMAIN && !demodulator)
            throw cRuntimeError("Demodulator not configured");
        if (levelOfDetail >= SAMPLE_DOMAIN && !pulseFilter)
            throw cRuntimeError("Pulse filter not configured");
    }
}

APSKRadioFrame *APSKLayeredReceiver::deserialize(const BitVector *bits) const
{
    Ieee80211Serializer deserializer;
    const std::vector<uint8>& bytes = bits->getFields();
    uint16_t macFrameLength = (bytes[0] << 8) + bytes[1];
    uint32_t crc = (bytes[2] << 24) + (bytes[3] << 16) + (bytes[4] << 8) + bytes[5];
    unsigned char *buffer = new unsigned char[macFrameLength];
    for (unsigned int i = 0; i < macFrameLength; i++)
        buffer[i] = bytes[i + 6];
    uint32_t computedCrc = ethernetCRC(buffer, macFrameLength / 2); // KLUDGE: remove the / 2
    cPacket *macFrame = nullptr;
    APSKRadioFrame *radioFrame = new APSKRadioFrame();
    if (crc != computedCrc) {
        EV_DEBUG << "CRC check failed" << endl;
        macFrame = new cPacket();
        radioFrame->setBitError(true);
    }
    else {
        try {
            macFrame = deserializer.parse(buffer, macFrameLength);
        }
        catch (cRuntimeError& e) {
            EV_ERROR << "Deserializing packet failed" << endl;
            macFrame = new cPacket();
            radioFrame->setBitError(true);
        }
    }
    delete [] buffer;
    radioFrame->setByteLength(6);
    radioFrame->encapsulate(macFrame);
    return radioFrame;
}

const IReceptionDecision *APSKLayeredReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const IRadio *receiver = reception->getReceiver();
    const IRadioMedium *medium = receiver->getMedium();
    const LayeredReception *layeredReception = dynamic_cast<const LayeredReception*>(reception);
    const LayeredTransmission *layeredTransmission = dynamic_cast<const LayeredTransmission*>(reception->getTransmission());
    const IReceptionAnalogModel *analogModel = layeredReception->getAnalogModel();
    const IReceptionSampleModel *sampleModel = nullptr;
    const IReceptionSymbolModel *symbolModel = nullptr;
    const IReceptionBitModel *bitModel = nullptr;
    const IReceptionPacketModel *packetModel = nullptr;
    const ISNIR *snir = medium->getSNIR(receiver, layeredTransmission);
    RadioReceptionIndication *receptionIndication = new RadioReceptionIndication();
    receptionIndication->setMinSNIR(snir->getMin());
    if (analogDigitalConverter)
    {
//        const IReceptionAnalogModel *totalAnalogModel = nullptr; // TODO: interference + receptionAnalogModel;
//        sampleModel = analogDigitalConverter->convertAnalogToDigital(totalAnalogModel);
    }
    if (levelOfDetail >= SAMPLE_DOMAIN) {
        if (!sampleModel)
            sampleModel = errorModel->computeSampleModel(layeredTransmission, snir);
        symbolModel = pulseFilter->filter(sampleModel);
    }
    if (levelOfDetail >= SYMBOL_DOMAIN) {
        if (!symbolModel)
            symbolModel = errorModel->computeSymbolModel(layeredTransmission, snir);
        bitModel = demodulator->demodulate(symbolModel);
    }
    if (levelOfDetail >= BIT_DOMAIN) {
        if (!bitModel)
            bitModel = errorModel->computeBitModel(layeredTransmission, snir);
        packetModel = decoder->decode(bitModel);
    }
    receptionIndication->setPacketErrorRate(packetModel->getPER());
    const BitVector* serializedPacket = packetModel->getSerializedPacket();
    cPacket *deserializedPacket = deserialize(serializedPacket);
    cPacket *macFrame = deserializedPacket->decapsulate();
    const ReceptionPacketModel *receptionPacketModel = new ReceptionPacketModel(macFrame, serializedPacket, NULL, NULL, NULL, 0, true);
    return new LayeredReceptionDecision(reception, receptionIndication, receptionPacketModel, nullptr, nullptr, nullptr, nullptr, true, true, receptionPacketModel->isPacketErrorless());
}

const IListening* APSKLayeredReceiver::createListening(const IRadio* radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, bandwidth);
}

// TODO: copy
const IListeningDecision* APSKLayeredReceiver::computeListeningDecision(const IListening* listening, const IInterference* interference) const
{
    const IRadio *receiver = listening->getReceiver();
    const IRadioMedium *radioMedium = receiver->getMedium();
    const IAnalogModel *analogModel = radioMedium->getAnalogModel();
    const INoise *noise = analogModel->computeNoise(listening, interference);
    const NarrowbandNoiseBase *flatNoise = check_and_cast<const NarrowbandNoiseBase *>(noise);
    W maxPower = flatNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    bool isListeningPossible = maxPower >= energyDetection;
    delete noise;
    EV_DEBUG << "Computing listening possible: maximum power = " << maxPower << ", energy detection = " << energyDetection << " -> listening is " << (isListeningPossible ? "possible" : "impossible") << endl;
    return new ListeningDecision(listening, isListeningPossible);
}

// TODO: this is not purely functional, see interface comment
// TODO: copy
bool APSKLayeredReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const LayeredReception *scalarReception = check_and_cast<const LayeredReception *>(reception);
    // TODO: scalar
    const ScalarReceptionSignalAnalogModel *analogModel = dynamic_cast<const ScalarReceptionSignalAnalogModel*>(scalarReception->getAnalogModel());
    if (bandListening->getCarrierFrequency() != analogModel->getCarrierFrequency() || bandListening->getBandwidth() != analogModel->getBandwidth()) {
        EV_DEBUG << "Computing reception possible: listening and reception bands are different -> reception is impossible" << endl;
        return false;
    }
    else {
        W minReceptionPower = scalarReception->computeMinPower(reception->getStartTime(), reception->getEndTime());
        bool isReceptionPossible = minReceptionPower >= sensitivity;
        EV_DEBUG << "Computing reception possible: minimum reception power = " << minReceptionPower << ", sensitivity = " << sensitivity << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;
        return isReceptionPossible;
    }
}

} // namespace physicallayer

} // namespace inet

