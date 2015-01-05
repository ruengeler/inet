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

#include "inet/physicallayer/apsk/layered/APSKReceiver.h"
#include "inet/physicallayer/contract/layered/ISymbol.h"
#include "inet/physicallayer/layered/LayeredReceptionDecision.h"
#include "inet/physicallayer/layered/LayeredReception.h"
#include "inet/physicallayer/layered/SignalSymbolModel.h"
#include "inet/physicallayer/layered/SignalSampleModel.h"
#include "inet/physicallayer/layered/SignalBitModel.h"
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

namespace inet {

namespace physicallayer {

#define SYMBOL_SIZE 48
#define ENCODED_SIGNAL_FIELD_LENGTH 48
// Table L-7—Bit assignment for SIGNAL field
#define SIGNAL_LENGTH_FIELD_START 5
#define SIGNAL_LENGTH_FIELD_END 16
#define SIGNAL_PARITY_FIELD 17
#define PPDU_SERVICE_FIELD_BITS_LENGTH 16
#define PPDU_TAIL_BITS_LENGTH 6

Define_Module(APSKReceiver);

APSKReceiver::APSKReceiver() :
    levelOfDetail((LevelOfDetail)-1),
    errorModel(nullptr),
    decoder(nullptr),
    headerDecoder(nullptr),
    demodulator(nullptr),
    headerDemodulator(nullptr),
    pulseFilter(nullptr),
    analogDigitalConverter(nullptr),
    energyDetection(W(NaN)),
    sensitivity(W(NaN)),
    carrierFrequency(Hz(NaN)),
    bandwidth(Hz(NaN)),
    snirThreshold(NaN)
{
}

void APSKReceiver::initialize(int stage)
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

unsigned int APSKReceiver::getSignalFieldLength(const BitVector *signalField) const
{
    ShortBitVector length;
    for (int i = SIGNAL_LENGTH_FIELD_START; i <= SIGNAL_LENGTH_FIELD_END; i++)
        length.appendBit(signalField->getBit(i));
    return length.toDecimal();
}

unsigned int APSKReceiver::calculatePadding(unsigned int dataFieldLengthInBits, const APSKModulationBase *modulationScheme, const ConvolutionalCode *fec) const
{
    ASSERT(modulationScheme != nullptr);
    unsigned int codedBitsPerSymbol = modulationScheme->getCodeWordLength() * SYMBOL_SIZE;
    unsigned int dataBitsPerSymbol = codedBitsPerSymbol * fec->getCodeRatePuncturingK() / fec->getCodeRatePuncturingN();
    return dataBitsPerSymbol - dataFieldLengthInBits % dataBitsPerSymbol;
}


const IReceptionSymbolModel* APSKReceiver::createSignalFieldReceptionSymbolModel(const IReceptionSymbolModel* receptionSymbolModel) const
{
//    return new ReceptionSymbolModel(transmissionSymbolModel->getSymbolLength(), transmissionSymbolModel->getSymbolRate(), corruptedSymbols);
    const std::vector<const ISymbol *> *symbols = receptionSymbolModel->getSymbols();
    std::vector<const ISymbol *> *signalSymbols = new std::vector<const ISymbol *>(); // FIXME: memory leak
    signalSymbols->push_back(symbols->at(0)); // The first symbol is the signal field symbol
    // TODO: revise symbolLength, symbolRate  !!
    return new ReceptionSymbolModel(1, receptionSymbolModel->getSymbolRate(), signalSymbols);
}

const IReceptionSymbolModel* APSKReceiver::createDataFieldReceptionSymbolModel(const IReceptionSymbolModel* receptionSymbolModel) const
{
    const std::vector<const ISymbol *> *symbols = receptionSymbolModel->getSymbols();
    std::vector<const ISymbol *> *dataSymbols = new std::vector<const ISymbol *>(); // FIXME: memory leak
    for (unsigned int i = 1; i < symbols->size(); i++)
        dataSymbols->push_back(symbols->at(i));
    // TODO: revise symbolLength, symbolRate  !!
    return new ReceptionSymbolModel(symbols->size() - 1, receptionSymbolModel->getSymbolRate(), dataSymbols);
}


const IReceptionBitModel* APSKReceiver::createSignalFieldReceptionBitModel(const IReceptionBitModel* receptionBitModel) const
{
    BitVector *headerBits = new BitVector();
    const BitVector *bits = receptionBitModel->getBits();
    for (unsigned int i = 0; i < ENCODED_SIGNAL_FIELD_LENGTH; i++)
        headerBits->appendBit(bits->getBit(i));
    return new ReceptionBitModel(ENCODED_SIGNAL_FIELD_LENGTH, -1, receptionBitModel->getHeaderBitRate(), -1, headerBits, receptionBitModel->getModulation());
}

const IReceptionBitModel* APSKReceiver::createDataFieldReceptionBitModel(const APSKModulationBase *demodulationScheme, const ConvolutionalCode *convCode, const IReceptionBitModel* receptionBitModel, const IReceptionPacketModel *signalFieldReceptionPacketModel) const
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
    return new ReceptionBitModel(-1, encodedDataFieldLengthInBits, -1, receptionBitModel->getPayloadBitRate(), dataBits, receptionBitModel->getModulation());
}

const IReceptionPacketModel *APSKReceiver::demodulateAndDecodeSignalField(const IRadioMedium *medium, const IRadio *receiver, const LayeredTransmission *transmission, const IReceptionSymbolModel *&receptionSymbolModel, const IReceptionBitModel *&receptionBitModel) const
{
    const IReceptionSymbolModel *signalFieldReceptionSymbolModel = nullptr;
    const IReceptionBitModel *signalFieldReceptionBitModel = nullptr;
    const IReceptionPacketModel *signalFieldReceptionPacketModel = nullptr;
    if (levelOfDetail >= SYMBOL_DOMAIN)
    {
        if (!receptionSymbolModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            ASSERT(transmission->getSymbolModel() != nullptr);
            receptionSymbolModel = errorModel->computeSymbolModel(transmission, snir);
        }
        signalFieldReceptionSymbolModel = createSignalFieldReceptionSymbolModel(receptionSymbolModel);
        signalFieldReceptionBitModel = headerDemodulator->demodulate(signalFieldReceptionSymbolModel);
    }
    if (levelOfDetail >= BIT_DOMAIN)
    {
        if (!signalFieldReceptionBitModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            ASSERT(transmission->getBitModel() != nullptr);
            receptionBitModel = errorModel->computeBitModel(transmission, snir); // TODO:
            signalFieldReceptionBitModel = createSignalFieldReceptionBitModel(receptionBitModel);
        }
        signalFieldReceptionPacketModel = decoder->decode(signalFieldReceptionBitModel);
    }
    return signalFieldReceptionPacketModel;
}

const IReceptionPacketModel* APSKReceiver::demodulateAndDecodeDataField(const IReceptionSymbolModel* receptionSymbolModel, const IReceptionBitModel* receptionBitModel, const IReceptionPacketModel *signalFieldReceptionPacketModel) const
{
    const IReceptionBitModel *dataFieldReceptionBitModel = nullptr;
    const IReceptionSymbolModel *dataFieldReceptionSymbolModel = nullptr;
    const IReceptionPacketModel *dataFieldReceptionPacketModel = nullptr;
    const BitVector *serializedSignalField = signalFieldReceptionPacketModel->getSerializedPacket();
    const APSKModulationBase *dataDemodulationScheme = nullptr; // TODO: ofdmModulation.getModulationScheme();
    if (levelOfDetail >= SYMBOL_DOMAIN)
    {
        dataFieldReceptionSymbolModel = createDataFieldReceptionSymbolModel(receptionSymbolModel);
        dataFieldReceptionBitModel = demodulator->demodulate(dataFieldReceptionSymbolModel);
        dataDemodulationScheme = nullptr; // TODO: demodulator->getDemodulationScheme();
    }
    if (levelOfDetail >= BIT_DOMAIN)
    {
        const APSKCode *code = nullptr;
        if (!decoder || !dataFieldReceptionBitModel)
            code = nullptr; // new APSKCode();
        if (!dataFieldReceptionBitModel)
            dataFieldReceptionBitModel = createDataFieldReceptionBitModel(dataDemodulationScheme, code->getConvolutionalCode(), receptionBitModel, signalFieldReceptionPacketModel);
        dataFieldReceptionPacketModel = decoder->decode(dataFieldReceptionBitModel);
        delete code;
    }
    return dataFieldReceptionPacketModel;
}

const IReceptionPacketModel* APSKReceiver::createCompleteReceptionPacketModel(const IReceptionPacketModel* signalFieldReceptionPacketModel, const IReceptionPacketModel* dataFieldReceptionPacketModel) const
{
    const BitVector *headerBits = signalFieldReceptionPacketModel->getSerializedPacket();
    BitVector *mergedBits = new BitVector(*headerBits);
    const BitVector *dataBits = dataFieldReceptionPacketModel->getSerializedPacket();
    for (unsigned int i = 0; i < dataBits->getSize(); i++)
        mergedBits->appendBit(dataBits->getBit(i));
    // TODO: deserializer
    cPacket *deserializedPacket = nullptr;
    return new ReceptionPacketModel(deserializedPacket, mergedBits, nullptr, nullptr, nullptr, 0, true);
}

const IReceptionDecision *APSKReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const IRadio *receiver = reception->getReceiver();
    const IRadioMedium *medium = receiver->getMedium();
    const LayeredTransmission *transmission = dynamic_cast<const LayeredTransmission*>(reception->getTransmission());
    const LayeredReception *layeredReception = dynamic_cast<const LayeredReception*>(reception);
    const IReceptionAnalogModel *receptionAnalogModel = layeredReception->getAnalogModel();
    if (!receptionAnalogModel)
        throw cRuntimeError("Reception analog model is obligatory");
    const IReceptionSampleModel *receptionSampleModel = nullptr;
    const IReceptionSymbolModel *receptionSymbolModel = nullptr;
    const IReceptionBitModel *receptionBitModel = nullptr;
    const IReceptionPacketModel *receptionPacketModel = nullptr;
    RadioReceptionIndication *receptionIndication = new RadioReceptionIndication();
    if (analogDigitalConverter)
    {
//        const IReceptionAnalogModel *totalAnalogModel = nullptr; // TODO: interference + receptionAnalogModel;
//        receptionSampleModel = analogDigitalConverter->convertAnalogToDigital(receptionAnalogModel, snir);
    }
    if (pulseFilter)
    {
        if (!receptionSampleModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            receptionIndication->setMinSNIR(snir->getMin());
            ASSERT(transmission->getSampleModel() != nullptr);
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
    // FIXME: Kludge: we have no serializer yet
    const cPacket *macFrame = transmission->getMacFrame();
    const ReceptionPacketModel *hackedPacketModel = new ReceptionPacketModel(macFrame, receptionPacketModel->getSerializedPacket(), receptionPacketModel->getForwardErrorCorrection(),
                                                                              receptionPacketModel->getScrambling(), receptionPacketModel->getInterleaving(),
                                                                              receptionPacketModel->getPER(), receptionPacketModel->isPacketErrorless());
    return new LayeredReceptionDecision(reception, receptionIndication, hackedPacketModel, nullptr, nullptr, nullptr, nullptr, true, true, hackedPacketModel->isPacketErrorless());
}

const IListening* APSKReceiver::createListening(const IRadio* radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, bandwidth);
}

// TODO: copy
const IListeningDecision* APSKReceiver::computeListeningDecision(const IListening* listening, const IInterference* interference) const
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
bool APSKReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception) const
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

