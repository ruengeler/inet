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

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/apsk/layered/APSKLayeredTransmitter.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/contract/layered/ISignalAnalogModel.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"
#include "inet/physicallayer/apsk/layered/APSKEncoder.h"
#include "inet/physicallayer/apsk/layered/APSKModulator.h"
#include "inet/physicallayer/apsk/layered/APSKRadioFrame_m.h"
#include "inet/physicallayer/layered/LayeredTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKLayeredTransmitter);

APSKLayeredTransmitter::APSKLayeredTransmitter() :
    levelOfDetail((LevelOfDetail)-1),
    encoder(nullptr),
    modulator(nullptr),
    pulseShaper(nullptr),
    digitalAnalogConverter(nullptr),
    bitrate(bps(NaN)),
    bandwidth(Hz(NaN)),
    carrierFrequency(Hz(NaN)),
    power(W(NaN))
{
}

void APSKLayeredTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        encoder = dynamic_cast<const IEncoder *>(getSubmodule("encoder"));
        modulator = dynamic_cast<const IModulator *>(getSubmodule("modulator"));
        pulseShaper = dynamic_cast<const IPulseShaper *>(getSubmodule("pulseShaper"));
        digitalAnalogConverter = dynamic_cast<const IDigitalAnalogConverter *>(getSubmodule("digitalAnalogConverter"));
        power = W(par("power"));
        bandwidth = Hz(par("bandwidth"));
        carrierFrequency = Hz(par("carrierFrequency"));
        bitrate = bps(par("bitrate"));

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

// FIXME: Kludge
BitVector *APSKLayeredTransmitter::serialize(const cPacket* packet) const
{
    // HACK: Here we just compute the bit-correct PLCP header
    // and then we fill the remaining with random bits
    const APSKRadioFrame *phyFrame = check_and_cast<const APSKRadioFrame*>(packet);
    BitVector *serializedPacket = new BitVector();
    // Reserved, 1 bit
    serializedPacket->appendBit(0); // Bit 4 is reserved. It shall be set to 0 on transmit and ignored on receive.
    // Length, 12 bits
    ShortBitVector byteLength(phyFrame->getByteLength(), 12); // == macFrame->getByteLength()
    for (unsigned int i = 0; i < byteLength.getSize(); i++)
        serializedPacket->appendBit(byteLength.getBit(i));
    // Parity, 1 bit
    serializedPacket->appendBit(0); // whatever (at least for now)
    // Tail, 6 bit
    serializedPacket->appendBit(0, 6); // The bits 18–23 constitute the SIGNAL TAIL field, and all 6 bits shall be set to 0
    // Service, 16 bit
    // The bits from 0–6 of the SERVICE field, which are transmitted first, are set to 0s
    // and are used to synchronize the descrambler in the receiver. The remaining 9 bits
    // (7–15) of the SERVICE field shall be reserved for future use. All reserved bits shall
    // be set to 0.
    serializedPacket->appendBit(0, 16);
    ASSERT(serializedPacket->getSize() == 40);
    for (unsigned int i = 0; i < byteLength.toDecimal() * 8; i++)
        serializedPacket->appendBit(intuniform(0,1));
    serializedPacket->appendBit(0, 6); // tail bits
    int dataBitsLength = 6 + 16 + byteLength.toDecimal() * 8;
    padding(serializedPacket, dataBitsLength);
    return serializedPacket;
}


const ITransmissionPacketModel* APSKLayeredTransmitter::createPacketModel(const cPacket* macFrame) const
{
    const RadioTransmissionRequest *controlInfo = dynamic_cast<const RadioTransmissionRequest *>(macFrame->getControlInfo());
    bps currentBitrate;
    if (controlInfo)
        currentBitrate = controlInfo->getBitrate();
    else
        currentBitrate = bitrate;
    // The PCLP header is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1),
    // Tail (6) and SERVICE (16) fields.
    int plcpHeaderLength = 4 + 1 + 12 + 1 + 6 + 16;
    APSKRadioFrame * phyFrame = new APSKRadioFrame();
    phyFrame->setByteLength(macFrame->getByteLength());
    phyFrame->encapsulate(macFrame->dup()); // TODO: fix this memory leak
    phyFrame->setBitLength(phyFrame->getBitLength() + plcpHeaderLength);
    const ITransmissionPacketModel *packetModel = new TransmissionPacketModel(phyFrame, nullptr);
    return packetModel;
}

const ITransmissionAnalogModel* APSKLayeredTransmitter::createAnalogModel(int headerBitLength, double headerBitRate, int payloadBitLength, double payloadBitRate) const
{
    simtime_t duration = headerBitLength / headerBitRate + payloadBitLength / payloadBitRate; // TODO: preamble duration
    const ITransmissionAnalogModel *transmissionAnalogModel = new ScalarTransmissionSignalAnalogModel(duration, power, carrierFrequency, bandwidth);
    return transmissionAnalogModel;
}

const ITransmissionPacketModel* APSKLayeredTransmitter::createSignalFieldPacketModel(const ITransmissionPacketModel* completePacketModel) const
{
    // The SIGNAL field is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1), Tail (6),
    // fields, so the SIGNAL field is 24 bits long.
    BitVector *signalField = new BitVector();
    const BitVector *serializedPacket = completePacketModel->getSerializedPacket();
    for (unsigned int i = 0; i < 24; i++)
        signalField->appendBit(serializedPacket->getBit(i));
    return new TransmissionPacketModel(nullptr, signalField);
}

const ITransmissionPacketModel* APSKLayeredTransmitter::createDataFieldPacketModel(const ITransmissionPacketModel* completePacketModel) const
{
    BitVector *dataField = new BitVector();
    const BitVector *serializedPacket = completePacketModel->getSerializedPacket();
    for (unsigned int i = 24; i < serializedPacket->getSize(); i++)
        dataField->appendBit(serializedPacket->getBit(i));
    return new TransmissionPacketModel(nullptr, dataField);
}

void APSKLayeredTransmitter::encodeAndModulate(const ITransmissionPacketModel* fieldPacketModel, const ITransmissionBitModel *&fieldBitModel, const ITransmissionSymbolModel *&fieldSymbolModel, const IEncoder *encoder, const IModulator *modulator, bool isSignalField) const
{
    if (levelOfDetail >= BIT_DOMAIN)
    {
        fieldBitModel = encoder->encode(fieldPacketModel);
    }
    if (levelOfDetail >= SYMBOL_DOMAIN)
    {
        if (fieldBitModel)
        {
            fieldSymbolModel = modulator->modulate(fieldBitModel);
        }
        else
            throw cRuntimeError("Modulator needs bit representation");
    }
}

const ITransmissionSymbolModel* APSKLayeredTransmitter::createSymbolModel(
        const ITransmissionSymbolModel* signalFieldSymbolModel,
        const ITransmissionSymbolModel* dataFieldSymbolModel) const
{
    const std::vector<const ISymbol *> *dataSymbols = dataFieldSymbolModel->getSymbols();
    std::vector<const ISymbol *> *mergedSymbols = new std::vector<const ISymbol *>(*signalFieldSymbolModel->getSymbols());
    for (unsigned int i = 0; i < dataSymbols->size(); i++)
        mergedSymbols->push_back(dataSymbols->at(i));
    // FIXME
    return new TransmissionSymbolModel(0, 0, mergedSymbols, dataFieldSymbolModel->getModulation());
}

const ITransmissionBitModel* APSKLayeredTransmitter::createBitModel(
        const ITransmissionBitModel* signalFieldBitModel,
        const ITransmissionBitModel* dataFieldBitModel) const
{
    BitVector *encodedBits = new BitVector(*signalFieldBitModel->getBits());
    unsigned int signalBitLength = signalFieldBitModel->getBits()->getSize();
    const BitVector *dataFieldBits = dataFieldBitModel->getBits();
    unsigned int dataBitLength = dataFieldBits->getSize();
    for (unsigned int i = 0; i < dataFieldBits->getSize(); i++)
        encodedBits->appendBit(dataFieldBits->getBit(i));
    bps signalBitrate = bps(0); // TODO: modulation->getBitrate();
    bps dataBitrate;
    if (modulator)
        dataBitrate = bitrate;
    else
    {
        dataBitrate = bps(0); // TODO: modulation->getBitrate();
    }
    return new TransmissionBitModel(signalBitLength, dataBitLength, signalBitrate.get(), dataBitrate.get(), encodedBits, dataFieldBitModel->getForwardErrorCorrection(), dataFieldBitModel->getScrambling(), dataFieldBitModel->getInterleaving());
}

void APSKLayeredTransmitter::padding(BitVector* serializedPacket, unsigned int dataBitsLength) const
{
    unsigned int codedBitsPerSymbol;
    const IInterleaving *interleaving = nullptr;
    const ConvolutionalCode *fec = nullptr;
    if (encoder)
    {
        const APSKEncoder *encoderModule = check_and_cast<const APSKEncoder *>(encoder);
        const APSKCode *code = encoderModule->getCode();
        ASSERT(code != nullptr);
        interleaving = code->getInterleaving();
        fec = code->getConvolutionalCode();
    }
    const APSKModulationBase *modulationScheme = nullptr; // TODO: modulation->getModulationScheme();
    codedBitsPerSymbol = modulationScheme->getCodeWordLength() * 48;
    const APSKCode code(nullptr, nullptr, nullptr);
    fec = code.getConvolutionalCode();
    unsigned int dataBitsPerSymbol = codedBitsPerSymbol * fec->getCodeRatePuncturingK() / fec->getCodeRatePuncturingN();
    unsigned int appendedBitsLength = dataBitsPerSymbol - dataBitsLength % dataBitsPerSymbol;
    serializedPacket->appendBit(0, appendedBitsLength);
}

const ITransmission *APSKLayeredTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const ITransmissionPacketModel *packetModel = createPacketModel(macFrame);
    BitVector *serializedPacket = serialize(packetModel->getPacket());
    const ITransmissionPacketModel *completePacketModel = new TransmissionPacketModel(packetModel->getPacket(), serializedPacket);
    delete packetModel;
    ASSERT(packetModel != nullptr);
    const ITransmissionBitModel *signalFieldBitModel = nullptr;
    const ITransmissionBitModel *dataFieldBitModel = nullptr;
    const ITransmissionSampleModel *sampleModel = nullptr;
    const ITransmissionSymbolModel *signalFieldSymbolModel = nullptr;
    const ITransmissionSymbolModel *dataFieldSymbolModel = nullptr;
    const ITransmissionPacketModel *signalFieldPacketModel = createSignalFieldPacketModel(completePacketModel);
    const ITransmissionPacketModel *dataFieldPacketModel = createDataFieldPacketModel(completePacketModel);
    encodeAndModulate(signalFieldPacketModel, signalFieldBitModel, signalFieldSymbolModel, encoder, modulator, true);
    encodeAndModulate(dataFieldPacketModel, dataFieldBitModel, dataFieldSymbolModel, encoder, modulator, false);
    const ITransmissionBitModel *bitModel = nullptr;
    if (levelOfDetail >= BIT_DOMAIN)
        bitModel = createBitModel(signalFieldBitModel, dataFieldBitModel);
    const ITransmissionSymbolModel *symbolModel = nullptr;
    if (levelOfDetail >= SYMBOL_DOMAIN)
        symbolModel = createSymbolModel(signalFieldSymbolModel, dataFieldSymbolModel);
    if (levelOfDetail >= SAMPLE_DOMAIN)
    {
        if (symbolModel)
        {
            sampleModel = pulseShaper->shape(symbolModel);
        }
        else
            throw cRuntimeError("Pulse shaper needs symbol representation");
    }
    const ITransmissionAnalogModel *analogModel = nullptr;
    if (digitalAnalogConverter)
    {
        if (!sampleModel)
            analogModel = digitalAnalogConverter->convertDigitalToAnalog(sampleModel);
        else
            throw cRuntimeError("Digital/analog converter needs sample representation");
    }
    else // Analog model is obligatory
    {
        ASSERT(bitModel != nullptr);
        analogModel = createAnalogModel(bitModel->getHeaderBitLength(), bitModel->getHeaderBitRate(), bitModel->getPayloadBitLength(), bitModel->getPayloadBitRate());; // FIXME
    }

    IMobility *mobility = transmitter->getAntenna()->getMobility();
    // assuming movement and rotation during transmission is negligible
    const simtime_t endTime = startTime + analogModel->getDuration();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new LayeredTransmission(completePacketModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation);
}

} // namespace physicallayer

} // namespace inet

