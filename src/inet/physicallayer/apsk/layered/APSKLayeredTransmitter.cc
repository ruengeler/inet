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
#include "inet/physicallayer/common/layered/SignalPacketModel.h"
#include "inet/physicallayer/contract/layered/ISignalAnalogModel.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"
#include "inet/physicallayer/apsk/layered/APSKEncoder.h"
#include "inet/physicallayer/apsk/layered/APSKModulator.h"
#include "inet/physicallayer/common/layered/LayeredTransmission.h"
#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211Serializer.h"
#include "inet/common/serializer/headerserializers/EthernetCRC.h"

namespace inet {

namespace physicallayer {

using namespace inet::serializer;

Define_Module(APSKLayeredTransmitter);

APSKLayeredTransmitter::APSKLayeredTransmitter() :
    levelOfDetail((LevelOfDetail)-1),
    encoder(nullptr),
    modulator(nullptr),
    pulseShaper(nullptr),
    digitalAnalogConverter(nullptr),
    power(W(NaN)),
    bitrate(bps(NaN)),
    bandwidth(Hz(NaN)),
    carrierFrequency(Hz(NaN))
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
        bitrate = bps(par("bitrate"));
        bandwidth = Hz(par("bandwidth"));
        carrierFrequency = Hz(par("carrierFrequency"));
        const char *levelOfDetailStr = par("levelOfDetail").stringValue();
        if (strcmp("bit", levelOfDetailStr) == 0)
            levelOfDetail = BIT_DOMAIN;
        else if (strcmp("symbol", levelOfDetailStr) == 0)
            levelOfDetail = SYMBOL_DOMAIN;
        else if (strcmp("sample", levelOfDetailStr) == 0)
            levelOfDetail = SAMPLE_DOMAIN;
        else
            throw cRuntimeError("Unknown level of detail='%s'", levelOfDetailStr);
        if (levelOfDetail >= BIT_DOMAIN && !encoder)
            throw cRuntimeError("Encoder not configured");
        if (levelOfDetail >= SYMBOL_DOMAIN && !modulator)
            throw cRuntimeError("Modulator not configured");
        if (levelOfDetail >= SAMPLE_DOMAIN && !pulseShaper)
            throw cRuntimeError("Pulse shaper not configured");
    }
}

const ITransmissionPacketModel* APSKLayeredTransmitter::createPacketModel(const cPacket* macFrame) const
{
    APSKRadioFrame * radioFrame = new APSKRadioFrame();
    radioFrame->setByteLength(4);
    radioFrame->encapsulate(macFrame->dup()); // TODO: fix this memory leak
    BitVector *bits = serialize(radioFrame);
    return new TransmissionPacketModel(radioFrame, bits);
}

const ITransmissionAnalogModel* APSKLayeredTransmitter::createAnalogModel(const ITransmissionBitModel *bitModel) const
{
    simtime_t duration = bitModel->getBits()->getSize() / bitrate.get();
    return new ScalarTransmissionSignalAnalogModel(duration, power, carrierFrequency, bandwidth);
}

BitVector *APSKLayeredTransmitter::serialize(const APSKRadioFrame *radioFrame) const
{
    Ieee80211Frame *macFrame = check_and_cast<Ieee80211Frame*>(radioFrame->getEncapsulatedPacket());
    uint16_t macFrameLength = macFrame->getByteLength();
    BitVector *bits = new BitVector();
    bits->appendByte(macFrameLength >> 8);
    bits->appendByte(macFrameLength >> 0);
    Ieee80211Serializer ieee80211Serializer;
    unsigned char *buffer = new unsigned char[macFrameLength + 10000]; // KLUDGE: serializer vs macFrameLength sux
    unsigned int serializedLength = ieee80211Serializer.serialize(macFrame, buffer, macFrameLength);
    // TODO: ASSERT(serializedLength == macFrameLength);
    uint32_t crc = ethernetCRC(buffer, macFrameLength / 2); // KLUDGE: remove the / 2
    bits->appendByte(crc >> 24);
    bits->appendByte(crc >> 16);
    bits->appendByte(crc >> 8);
    bits->appendByte(crc >> 0);
    for (unsigned int i = 0; i < serializedLength; i++)
        bits->appendByte(buffer[i]);
    delete [] buffer;
    return bits;
}

const ITransmission *APSKLayeredTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const ITransmissionPacketModel *packetModel = createPacketModel(macFrame);
    const ITransmissionBitModel *bitModel = nullptr;
    if (levelOfDetail >= BIT_DOMAIN)
        bitModel = encoder->encode(packetModel);
    const ITransmissionSymbolModel *symbolModel = nullptr;
    if (levelOfDetail >= SYMBOL_DOMAIN)
        symbolModel = modulator->modulate(bitModel);
    const ITransmissionSampleModel *sampleModel = nullptr;
    if (levelOfDetail >= SAMPLE_DOMAIN)
        sampleModel = pulseShaper->shape(symbolModel);
    const ITransmissionAnalogModel *analogModel = nullptr;
    if (digitalAnalogConverter) {
        if (sampleModel == nullptr)
            throw cRuntimeError("Digital analog converter needs sample domain representation");
        else
            analogModel = digitalAnalogConverter->convertDigitalToAnalog(sampleModel);
    }
    else
        analogModel = createAnalogModel(bitModel);
    // assuming movement and rotation during transmission is negligible
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const simtime_t endTime = startTime + analogModel->getDuration();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new LayeredTransmission(packetModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation);
}

} // namespace physicallayer

} // namespace inet

