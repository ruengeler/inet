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
#include "inet/physicallayer/ieee80211/layered/Ieee80211LayeredTransmitter.h"
#include "inet/physicallayer/layered/LayeredTransmission.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211PHYFrame_m.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211LayeredTransmitter);

Ieee80211LayeredTransmitter::Ieee80211LayeredTransmitter() :
    encoder(NULL),
    modulator(NULL),
    pulseShaper(NULL),
    digitalAnalogConverter(NULL)
{
}

void Ieee80211LayeredTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        encoder = check_and_cast<IEncoder *>(getSubmodule("encoder"));
        modulator = check_and_cast<IModulator *>(getSubmodule("modulator"));
        pulseShaper = check_and_cast<IPulseShaper *>(getSubmodule("pulseShaper"));
        digitalAnalogConverter = check_and_cast<IDigitalAnalogConverter *>(getSubmodule("digitalAnalogConverter"));
        power = W(par("power"));
        bandwidth = Hz(par("bandwidth"));
        carrierFrequency = Hz(par("carrierFrequency"));
        channelSpacing = Hz(par("channelSpacing"));
    }
}

const ITransmissionPacketModel* Ieee80211LayeredTransmitter::createPacketModel(const cPacket* macFrame) const
{
    const RadioTransmissionRequest *controlInfo = dynamic_cast<const RadioTransmissionRequest *>(macFrame->getControlInfo());
    bps bitRate = controlInfo->getBitrate();
    int rate = calculateRateField(channelSpacing, bitRate).toDecimal();
    // The PCLP header is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1),
    // Tail (6) and SERVICE (16) fields.
    int plcpHeaderLength = 4 + 1 + 12 + 1 + 6 + 16;
    Ieee80211PHYFrame * phyFrame = new Ieee80211PHYFrame();
    phyFrame->setRate(rate);
    phyFrame->setLength(macFrame->getByteLength());
    phyFrame->encapsulate(macFrame->dup()); // TODO: fix this memory leak
    phyFrame->setBitLength(phyFrame->getLength() + plcpHeaderLength);
    const ITransmissionPacketModel *packetModel = new TransmissionPacketModel(phyFrame);
    return packetModel;
}


ShortBitVector Ieee80211LayeredTransmitter::calculateRateField(Hz channelSpacing, bps bitrate) const
{
    if (channelSpacing == MHz(20))
    {
        if (bitrate == bps(6000000))
            return ShortBitVector("1101");
        else if (bitrate == bps(9000000))
            return ShortBitVector("1111");
        else if (bitrate == bps(12000000))
            return ShortBitVector("0101");
        else if (bitrate == bps(18000000))
            return ShortBitVector("0111");
        else if (bitrate == bps(24000000))
            return ShortBitVector("1001");
        else if (bitrate == bps(36000000))
            return ShortBitVector("1011");
        else if (bitrate == bps(48000000))
            return ShortBitVector("0001");
        else if (bitrate == bps(54000000))
            return ShortBitVector("0011");
        else
            throw cRuntimeError("%lf Hz channel spacing does not support %lf bps bitrate", channelSpacing.get(), bitrate.get());
    }
    else if (channelSpacing == MHz(10))
    {
        if (bitrate == bps(3000000))
            return ShortBitVector("1101");
        else if (bitrate == bps(4500000))
            return ShortBitVector("1111");
        else if (bitrate == bps(6000000))
            return ShortBitVector("0101");
        else if (bitrate == bps(9000000))
            return ShortBitVector("0111");
        else if (bitrate == bps(12000000))
            return ShortBitVector("1001");
        else if (bitrate == bps(18000000))
            return ShortBitVector("1011");
        else if (bitrate == bps(24000000))
            return ShortBitVector("0001");
        else if (bitrate == bps(27000000))
            return ShortBitVector("0011");
        else
            throw cRuntimeError("%lf Hz channel spacing does not support %lf bps bitrate", channelSpacing.get(), bitrate.get());
    }
    else if (channelSpacing == MHz(5))
    {
       if (bitrate == bps(1500000))
           return ShortBitVector("1101");
       else if (bitrate == bps(2250000))
           return ShortBitVector("1111");
       else if (bitrate == bps(3000000))
           return ShortBitVector("0101");
       else if (bitrate == bps(4500000))
           return ShortBitVector("0111");
       else if (bitrate == bps(6000000))
           return ShortBitVector("1001");
       else if (bitrate == bps(9000000))
           return ShortBitVector("1011");
       else if (bitrate == bps(12000000))
           return ShortBitVector("0001");
       else if (bitrate == bps(13500000))
           return ShortBitVector("0011");
       else
           throw cRuntimeError("%lf Hz channel spacing does not support %lf bps bitrate", channelSpacing.get(), bitrate.get());
    }
    else
        throw cRuntimeError("Unknown channel spacing = %lf", channelSpacing);
    return ShortBitVector("0000");
}

const ITransmission *Ieee80211LayeredTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const ITransmissionPacketModel *packetModel = createPacketModel(macFrame);
    const ITransmissionBitModel *bitModel = encoder->encode(packetModel);
    const ITransmissionSymbolModel *symbolModel = modulator->modulate(bitModel);
    const ITransmissionSampleModel *sampleModel = pulseShaper->shape(symbolModel);
    const ITransmissionAnalogModel *analogModel = digitalAnalogConverter->convertDigitalToAnalog(sampleModel);
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    // assuming movement and rotation during transmission is negligible
    const simtime_t endTime = startTime + analogModel->getDuration();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new LayeredTransmission(packetModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, bandwidth, carrierFrequency, power);
}

} // namespace physicallayer

} // namespace inet
