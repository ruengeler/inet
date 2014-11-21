//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/contract/ISNIR.h"
#include "inet/physicallayer/contract/IErrorModel.h"
#include "inet/physicallayer/base/NarrowbandNoiseBase.h"
#include "inet/physicallayer/layered/LayeredReceptionDecision.h"
#include "inet/physicallayer/layered/LayeredReceiver.h"
#include "inet/physicallayer/layered/LayeredScalarTransmission.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/layered/LayeredScalarReception.h"
#include "inet/physicallayer/layered/SignalSymbolModel.h"
#include "inet/physicallayer/layered/SignalSampleModel.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"
#include "inet/physicallayer/layered/SignalBitModel.h"
#include "inet/physicallayer/common/BandListening.h"
#include "inet/physicallayer/common/ListeningDecision.h"
#include "inet/physicallayer/contract/IRadioMedium.h"

namespace inet {

namespace physicallayer {

Define_Module(LayeredReceiver);

LayeredReceiver::LayeredReceiver() :
    errorModel(NULL),
    decoder(NULL),
    headerDecoder(NULL),
    demodulator(NULL),
    headerDemodulator(NULL),
    pulseFilter(NULL),
    analogDigitalConverter(NULL)
{
}

void LayeredReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        errorModel = dynamic_cast<ILayeredErrorModel *>(getSubmodule("errorModel"));
        decoder = dynamic_cast<IDecoder *>(getSubmodule("decoder"));
        headerDecoder = dynamic_cast<IDecoder *>(getSubmodule("headerDecoder"));
        demodulator = dynamic_cast<IDemodulator *>(getSubmodule("demodulator"));
        headerDemodulator = dynamic_cast<IDemodulator *>(getSubmodule("headerDemodulator"));
        pulseFilter = dynamic_cast<IPulseFilter *>(getSubmodule("pulseFilter"));
        analogDigitalConverter = dynamic_cast<IAnalogDigitalConverter *>(getSubmodule("analogDigitalConverter"));

        const char *levelOfDetailStr = par("levelOfDetail").stringValue();
        if (strcmp("bit", levelOfDetailStr) == 0)
            levelOfDetail = BIT_DOMAIN;
        else if (strcmp("symbol", levelOfDetailStr) == 0)
            levelOfDetail = SYMBOL_DOMAIN;
        else if (strcmp("sample", levelOfDetailStr) == 0)
            levelOfDetail = SAMPLE_DOMAIN;
        else
            throw cRuntimeError("Unknown level of detail='%s'", levelOfDetailStr);

        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        // TODO: temporary parameters
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
        snirThreshold = math::dB2fraction(par("snirThreshold"));
    }
}
// TODO: revise this function
const IReceptionDecision *LayeredReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const IRadio *receiver = reception->getReceiver();
    const IRadioMedium *medium = receiver->getMedium();
    const LayeredScalarTransmission *transmission = dynamic_cast<const LayeredScalarTransmission*>(reception->getTransmission());
    const LayeredScalarReception *layeredReception = dynamic_cast<const LayeredScalarReception*>(reception);
    const IReceptionAnalogModel *receptionAnalogModel = layeredReception->getAnalogModel();
    if (!receptionAnalogModel)
        throw cRuntimeError("Reception analog model is obligatory");
    const IReceptionSampleModel *receptionSampleModel = NULL;
    const IReceptionSymbolModel *receptionSymbolModel = NULL;
    const IReceptionBitModel *receptionBitModel = NULL;
    const IReceptionPacketModel *receptionPacketModel = NULL;
    RadioReceptionIndication *receptionIndication = new RadioReceptionIndication();
    if (analogDigitalConverter)
    {
//        const IReceptionAnalogModel *totalAnalogModel = NULL; // TODO: interference + receptionAnalogModel;
//        receptionSampleModel = analogDigitalConverter->convertAnalogToDigital(totalAnalogModel);
    }
    if (pulseFilter)
    {
        if (!receptionSampleModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            receptionIndication->setMinSNIR(snir->getMin());
            ASSERT(transmission->getSampleModel() != NULL);
            receptionSampleModel = errorModel->computeSampleModel(transmission, snir);
        }
        receptionIndication->setMinRSSI(receptionSampleModel->getRSSI());
        receptionSymbolModel = pulseFilter->filter(receptionSampleModel);
    }
    if (headerDemodulator)
    {
        if (!receptionSymbolModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            ASSERT(transmission->getSymbolModel() != NULL);
            receptionSymbolModel = errorModel->computeSymbolModel(transmission, snir);
        }
//        FIXME: delete ser from reception indication?
//        receptionIndication->setSymbolErrorCount(receptionSymbolModel->getSymbolErrorCount());
//        receptionIndication->setSymbolErrorRate(receptionSymbolModel->getSER());
        receptionBitModel = headerDemodulator->demodulate(receptionSymbolModel);
    }
    if (decoder)
    {
        if (!receptionBitModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            ASSERT(transmission->getBitModel() != NULL);
            receptionBitModel = errorModel->computeBitModel(transmission, snir);
        }
//        FIXME: delete ber from reception indication?
//        receptionIndication->setBitErrorCount(receptionBitModel->getBitErrorCount());
//        receptionIndication->setBitErrorRate(receptionBitModel->getBER());
        receptionPacketModel = decoder->decode(receptionBitModel);
    }
    if (!receptionPacketModel)
        throw cRuntimeError("Packet model is obligatory");
    receptionIndication->setPacketErrorRate(receptionPacketModel->getPER());
    // FIXME: Kludge: we have no serializer yet
    const cPacket *macFrame = transmission->getMacFrame();
    const ReceptionPacketModel *hackedPacketModel = new ReceptionPacketModel(macFrame, receptionPacketModel->getSerializedPacket(), receptionPacketModel->getForwardErrorCorrection(),
                                                                              receptionPacketModel->getScrambling(), receptionPacketModel->getInterleaving(),
                                                                              receptionPacketModel->getPER(), receptionPacketModel->isPacketErrorless());
    return new LayeredReceptionDecision(reception, receptionIndication, hackedPacketModel, NULL, NULL, NULL, NULL, true, true, true);
}

const IListening* LayeredReceiver::createListening(const IRadio* radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, bandwidth);
}

// TODO: copy
const IListeningDecision* LayeredReceiver::computeListeningDecision(const IListening* listening, const IInterference* interference) const
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
bool LayeredReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const NarrowbandReceptionBase *flatReception = check_and_cast<const NarrowbandReceptionBase *>(reception);
    if (bandListening->getCarrierFrequency() != flatReception->getCarrierFrequency() || bandListening->getBandwidth() != flatReception->getBandwidth()) {
        EV_DEBUG << "Computing reception possible: listening and reception bands are different -> reception is impossible" << endl;
        return false;
    }
    else {
        W minReceptionPower = flatReception->computeMinPower(reception->getStartTime(), reception->getEndTime());
        bool isReceptionPossible = minReceptionPower >= sensitivity;
        EV_DEBUG << "Computing reception possible: minimum reception power = " << minReceptionPower << ", sensitivity = " << sensitivity << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;
        return isReceptionPossible;
    }
}

} // namespace physicallayer

} // namespace inet
