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

#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMReceiver.h"
#include "inet/physicallayer/contract/layered/ISymbol.h"
#include "inet/physicallayer/common/layered/LayeredReceptionDecision.h"
#include "inet/physicallayer/common/layered/LayeredReception.h"
#include "inet/physicallayer/common/layered/SignalSymbolModel.h"
#include "inet/physicallayer/common/layered/SignalSampleModel.h"
#include "inet/physicallayer/common/layered/SignalBitModel.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"
#include "inet/physicallayer/common/BandListening.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMDecoderModule.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMDemodulatorModule.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/ieee80211/Ieee80211OFDMModulation.h"
#include "inet/physicallayer/base/NarrowbandNoiseBase.h"
#include "inet/physicallayer/common/ListeningDecision.h"
#include "inet/physicallayer/analogmodel/ScalarAnalogModel.h"
#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211PhySerializer.h"

namespace inet {
namespace physicallayer {

#define OFDM_SYMBOL_SIZE 48
#define ENCODED_SIGNAL_FIELD_LENGTH 48
// Table L-7—Bit assignment for SIGNAL field
#define SIGNAL_RATE_FIELD_START 0
#define SIGNAL_RATE_FIELD_END 3
#define SIGNAL_LENGTH_FIELD_START 5
#define SIGNAL_LENGTH_FIELD_END 16
#define SIGNAL_PARITY_FIELD 17
#define PPDU_SERVICE_FIELD_BITS_LENGTH 16
#define PPDU_TAIL_BITS_LENGTH 6

using namespace serializer;

Define_Module(Ieee80211OFDMReceiver);

Ieee80211OFDMReceiver::Ieee80211OFDMReceiver() :
    errorModel(NULL),
    decoder(NULL),
    headerDecoder(NULL),
    demodulator(NULL),
    headerDemodulator(NULL),
    pulseFilter(NULL),
    analogDigitalConverter(NULL)
{
}

void Ieee80211OFDMReceiver::initialize(int stage)
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

        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        // TODO: temporary parameters
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
        snirThreshold = math::dB2fraction(par("snirThreshold"));
        channelSpacing = Hz(par("channelSpacing"));

        const char *levelOfDetailStr = par("levelOfDetail").stringValue();
        if (strcmp("bit", levelOfDetailStr) == 0)
            levelOfDetail = BIT_DOMAIN;
        else if (strcmp("symbol", levelOfDetailStr) == 0)
            levelOfDetail = SYMBOL_DOMAIN;
        else if (strcmp("sample", levelOfDetailStr) == 0)
            levelOfDetail = SAMPLE_DOMAIN;
        else
            throw cRuntimeError("Unknown level of detail='%s'", levelOfDetailStr);
    }
}

uint8_t Ieee80211OFDMReceiver::getRate(const BitVector* serializedPacket) const
{
    ShortBitVector rate;
    for (unsigned int i = 0; i < 4; i++)
        rate.appendBit(serializedPacket->getBit(i));
    return rate.toDecimal();
}

unsigned int Ieee80211OFDMReceiver::getSignalFieldLength(const BitVector *signalField) const
{
    ShortBitVector length;
    for (int i = SIGNAL_LENGTH_FIELD_START; i <= SIGNAL_LENGTH_FIELD_END; i++)
        length.appendBit(signalField->getBit(i));
    return length.toDecimal();
}

unsigned int Ieee80211OFDMReceiver::calculatePadding(unsigned int dataFieldLengthInBits, const APSKModulationBase *modulationScheme, const ConvolutionalCode *fec) const
{
    ASSERT(modulationScheme != NULL);
    unsigned int codedBitsPerOFDMSymbol = modulationScheme->getCodeWordLength() * OFDM_SYMBOL_SIZE;
    unsigned int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol * fec->getCodeRatePuncturingK() / fec->getCodeRatePuncturingN();
    return dataBitsPerOFDMSymbol - dataFieldLengthInBits % dataBitsPerOFDMSymbol;
}

const IReceptionSymbolModel* Ieee80211OFDMReceiver::createSignalFieldReceptionSymbolModel(const IReceptionSymbolModel* receptionSymbolModel) const
{
//    return new ReceptionSymbolModel(transmissionSymbolModel->getSymbolLength(), transmissionSymbolModel->getSymbolRate(), corruptedSymbols);
    const std::vector<const ISymbol *> *symbols = receptionSymbolModel->getSymbols();
    std::vector<const ISymbol *> *signalSymbols = new std::vector<const ISymbol *>(); // FIXME: memory leak
    signalSymbols->push_back(symbols->at(0)); // The first symbol is the signal field symbol
    // TODO: revise symbolLength, symbolRate  !!
    return new ReceptionSymbolModel(1, receptionSymbolModel->getSymbolRate(), signalSymbols);
}

const IReceptionSymbolModel* Ieee80211OFDMReceiver::createDataFieldReceptionSymbolModel(const IReceptionSymbolModel* receptionSymbolModel) const
{
    const std::vector<const ISymbol *> *symbols = receptionSymbolModel->getSymbols();
    std::vector<const ISymbol *> *dataSymbols = new std::vector<const ISymbol *>(); // FIXME: memory leak
    for (unsigned int i = 1; i < symbols->size(); i++)
        dataSymbols->push_back(symbols->at(i));
    // TODO: revise symbolLength, symbolRate  !!
    return new ReceptionSymbolModel(symbols->size() - 1, receptionSymbolModel->getSymbolRate(), dataSymbols);
}


const IReceptionBitModel* Ieee80211OFDMReceiver::createSignalFieldReceptionBitModel(const IReceptionBitModel* receptionBitModel) const
{
    BitVector *headerBits = new BitVector();
    const BitVector *bits = receptionBitModel->getBits();
    for (unsigned int i = 0; i < ENCODED_SIGNAL_FIELD_LENGTH; i++)
        headerBits->appendBit(bits->getBit(i));
    return new ReceptionBitModel(ENCODED_SIGNAL_FIELD_LENGTH, -1, receptionBitModel->getHeaderBitRate(), -1, headerBits);
}

const IReceptionBitModel* Ieee80211OFDMReceiver::createDataFieldReceptionBitModel(const APSKModulationBase *demodulationScheme, const ConvolutionalCode *convCode, const IReceptionBitModel* receptionBitModel, const IReceptionPacketModel *signalFieldReceptionPacketModel) const
{
    unsigned int psduLengthInBits = getSignalFieldLength(signalFieldReceptionPacketModel->getSerializedPacket()) * 8;
    unsigned int dataFieldLengthInBits = psduLengthInBits + PPDU_SERVICE_FIELD_BITS_LENGTH + PPDU_TAIL_BITS_LENGTH;
    dataFieldLengthInBits += calculatePadding(dataFieldLengthInBits, demodulationScheme, convCode);
    ASSERT(dataFieldLengthInBits % convCode->getCodeRatePuncturingK() == 0);
    unsigned int encodedDataFieldLengthInBits = dataFieldLengthInBits * convCode->getCodeRatePuncturingN() / convCode->getCodeRatePuncturingK();
    const BitVector *bits = receptionBitModel->getBits();
    if (dataFieldLengthInBits + ENCODED_SIGNAL_FIELD_LENGTH > bits->getSize())
        throw cRuntimeError("The calculated data field length = %d is greater then the actual bitvector length = %d", dataFieldLengthInBits, bits->getSize());
    BitVector *dataBits = new BitVector();
    for (unsigned int i = 0; i < encodedDataFieldLengthInBits; i++)
        dataBits->appendBit(bits->getBit(ENCODED_SIGNAL_FIELD_LENGTH+i));
    return new ReceptionBitModel(-1, encodedDataFieldLengthInBits, -1, receptionBitModel->getPayloadBitRate(), dataBits);
}

const IReceptionPacketModel *Ieee80211OFDMReceiver::demodulateAndDecodeSignalField(const IRadioMedium *medium, const IRadio *receiver, const LayeredTransmission *transmission, const IReceptionSymbolModel *&receptionSymbolModel, const IReceptionBitModel *&receptionBitModel) const
{
    const IReceptionSymbolModel *signalFieldReceptionSymbolModel = NULL;
    const IReceptionBitModel *signalFieldReceptionBitModel = NULL;
    const IReceptionPacketModel *signalFieldReceptionPacketModel = NULL;
    if (levelOfDetail >= SYMBOL_DOMAIN)
    {
        if (!receptionSymbolModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            ASSERT(transmission->getSymbolModel() != NULL);
            receptionSymbolModel = errorModel->computeSymbolModel(transmission, snir);
        }
        signalFieldReceptionSymbolModel = createSignalFieldReceptionSymbolModel(receptionSymbolModel);
        if (headerDemodulator) // non-compliant mode
            signalFieldReceptionBitModel = headerDemodulator->demodulate(signalFieldReceptionSymbolModel);
        else
        {
            // In compliant mode, the signal field modulation is always BPSK
            const Ieee80211OFDMDemodulator demodulator(&BPSKModulation::singleton);
            signalFieldReceptionBitModel = demodulator.demodulate(signalFieldReceptionSymbolModel);
        }
    }
    if (levelOfDetail >= BIT_DOMAIN)
    {
        if (!signalFieldReceptionBitModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            ASSERT(transmission->getBitModel() != NULL);
            receptionBitModel = errorModel->computeBitModel(transmission, snir); // TODO:
            signalFieldReceptionBitModel = createSignalFieldReceptionBitModel(receptionBitModel);
        }
        if (headerDecoder) // non-compliant mode
            signalFieldReceptionPacketModel = decoder->decode(signalFieldReceptionBitModel);
        else
        {
            const Ieee80211OFDMCode *code = new Ieee80211OFDMCode(channelSpacing);
            const Ieee80211OFDMDecoder decoder(code);
            signalFieldReceptionPacketModel = decoder.decode(signalFieldReceptionBitModel);
        }
    }
    return signalFieldReceptionPacketModel;
}

const IReceptionPacketModel* Ieee80211OFDMReceiver::demodulateAndDecodeDataField(const IReceptionSymbolModel* receptionSymbolModel, const IReceptionBitModel* receptionBitModel, const IReceptionPacketModel *signalFieldReceptionPacketModel) const
{
    const IReceptionBitModel *dataFieldReceptionBitModel = NULL;
    const IReceptionSymbolModel *dataFieldReceptionSymbolModel = NULL;
    const IReceptionPacketModel *dataFieldReceptionPacketModel = NULL;
    const BitVector *serializedSignalField = signalFieldReceptionPacketModel->getSerializedPacket();
    uint8_t rate = getRate(serializedSignalField);
    const Ieee80211OFDMModulation ofdmModulation(rate, channelSpacing);
    const APSKModulationBase *dataDemodulationScheme = ofdmModulation.getModulationScheme();
    if (levelOfDetail >= SYMBOL_DOMAIN)
    {
        dataFieldReceptionSymbolModel = createDataFieldReceptionSymbolModel(receptionSymbolModel);
        if (demodulator) // non-compliant mode
        {
            dataFieldReceptionBitModel = demodulator->demodulate(dataFieldReceptionSymbolModel);
            const Ieee80211OFDMDemodulatorModule *ofdmDemodulator = check_and_cast<const Ieee80211OFDMDemodulatorModule *>(demodulator);
            dataDemodulationScheme = ofdmDemodulator->getDemodulationScheme();
        }
        else // compliant mode
        {
            const Ieee80211OFDMDemodulator ofdmDemodulator(dataDemodulationScheme);
            dataFieldReceptionBitModel = ofdmDemodulator.demodulate(dataFieldReceptionSymbolModel);
        }
    }
    if (levelOfDetail >= BIT_DOMAIN)
    {
        const Ieee80211OFDMCode *code = NULL;
        if (!decoder || !dataFieldReceptionBitModel)
            code = new Ieee80211OFDMCode(rate, channelSpacing);
        if (!dataFieldReceptionBitModel)
            dataFieldReceptionBitModel = createDataFieldReceptionBitModel(dataDemodulationScheme, code->getConvCode(), receptionBitModel, signalFieldReceptionPacketModel);
        if (decoder) // non-compliant mode
            dataFieldReceptionPacketModel = decoder->decode(dataFieldReceptionBitModel);
        else
        {
            const Ieee80211OFDMDecoder decoder(code);
            dataFieldReceptionPacketModel = decoder.decode(dataFieldReceptionBitModel);
        }
        delete code;
    }
    return dataFieldReceptionPacketModel;
}

const IReceptionPacketModel* Ieee80211OFDMReceiver::createCompleteReceptionPacketModel(const IReceptionPacketModel* signalFieldReceptionPacketModel, const IReceptionPacketModel* dataFieldReceptionPacketModel) const
{
    const BitVector *headerBits = signalFieldReceptionPacketModel->getSerializedPacket();
    BitVector *mergedBits = new BitVector(*headerBits);
    const BitVector *dataBits = dataFieldReceptionPacketModel->getSerializedPacket();
    for (unsigned int i = 0; i < dataBits->getSize(); i++)
        mergedBits->appendBit(dataBits->getBit(i));
    // NOTE: remove padding is not necessary
    Ieee80211PhySerializer deserializer;
    cPacket *deserializedPacket = deserializer.deserialize(mergedBits);
    cPacket *macFrame = deserializedPacket->decapsulate();
    return new ReceptionPacketModel(macFrame, mergedBits, NULL, NULL, NULL, 0, true); // TODO: fill the constructor with correct values or remove these fields from the reception packet model.
}

const IReceptionDecision *Ieee80211OFDMReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const IRadio *receiver = reception->getReceiver();
    const IRadioMedium *medium = receiver->getMedium();
    const LayeredTransmission *transmission = dynamic_cast<const LayeredTransmission*>(reception->getTransmission());
    const LayeredReception *layeredReception = dynamic_cast<const LayeredReception*>(reception);
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
//        receptionSampleModel = analogDigitalConverter->convertAnalogToDigital(receptionAnalogModel, snir);
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
    const IReceptionPacketModel *signalFieldReceptionPacketModel = demodulateAndDecodeSignalField(medium, receiver, transmission, receptionSymbolModel, receptionBitModel);
    const IReceptionPacketModel *dataFieldReceptionPacketModel = demodulateAndDecodeDataField(receptionSymbolModel, receptionBitModel, signalFieldReceptionPacketModel);
    receptionPacketModel = createCompleteReceptionPacketModel(signalFieldReceptionPacketModel, dataFieldReceptionPacketModel);
    if (!receptionPacketModel)
        throw cRuntimeError("Packet model is obligatory");
    receptionIndication->setPacketErrorRate(receptionPacketModel->getPER());
    return new LayeredReceptionDecision(reception, receptionIndication, receptionPacketModel, receptionBitModel, receptionSymbolModel, receptionSampleModel, receptionAnalogModel, true, true, receptionPacketModel->isPacketErrorless());
}

const IListening* Ieee80211OFDMReceiver::createListening(const IRadio* radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, bandwidth);
}

// TODO: copy
const IListeningDecision* Ieee80211OFDMReceiver::computeListeningDecision(const IListening* listening, const IInterference* interference) const
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
bool Ieee80211OFDMReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception) const
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

} /* namespace physicallayer */
} /* namespace inet */
