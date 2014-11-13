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

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/layered/LayeredTransmitter.h"
#include "inet/physicallayer/layered/LayeredTransmission.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"

namespace inet {

namespace physicallayer {

LayeredTransmitter::LayeredTransmitter() :
    encoder(NULL),
    modulator(NULL),
    pulseShaper(NULL),
    digitalAnalogConverter(NULL)
{
}

void LayeredTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        encoder = dynamic_cast<IEncoder *>(getSubmodule("encoder"));
        modulator = dynamic_cast<IModulator *>(getSubmodule("modulator"));
        pulseShaper = dynamic_cast<IPulseShaper *>(getSubmodule("pulseShaper"));
        digitalAnalogConverter = dynamic_cast<IDigitalAnalogConverter *>(getSubmodule("digitalAnalogConverter"));

        power = W(par("power"));
        bandwidth = Hz(par("bandwidth"));
        carrierFrequency = Hz(par("carrierFrequency"));
        bitrate = bps(par("bitrate"));
    }
}

const ITransmissionPacketModel* LayeredTransmitter::createPacketModel(const cPacket *macFrame) const
{
    const ITransmissionPacketModel *packetModel = new TransmissionPacketModel(macFrame);
    return packetModel;
}

const ITransmissionAnalogModel* LayeredTransmitter::createAnalogModel(int headerBitLength, double headerBitRate, int payloadBitLength, double payloadBitRate) const
{
    simtime_t duration = headerBitLength / headerBitRate + payloadBitLength / payloadBitRate; // TODO: preamble duration
    const ITransmissionAnalogModel *transmissionAnalogModel = new ScalarTransmissionSignalAnalogModel(duration, power, carrierFrequency, bandwidth);
    return transmissionAnalogModel;
}

const ITransmission *LayeredTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const ITransmissionPacketModel *packetModel = createPacketModel(macFrame);
    ASSERT(packetModel != NULL);
    const ITransmissionBitModel *bitModel = NULL;
    if (encoder)
        bitModel = encoder->encode(packetModel);
    const ITransmissionSymbolModel *symbolModel = NULL;
    if (modulator && bitModel)
        symbolModel = modulator->modulate(bitModel);
    else if (modulator && !bitModel)
        throw cRuntimeError("Modulators need bit representation");
    const ITransmissionSampleModel *sampleModel = NULL;
    if (pulseShaper && symbolModel)
        sampleModel = pulseShaper->shape(symbolModel);
    else if (pulseShaper && !sampleModel)
        throw cRuntimeError("Pulse shapers need symbol representation");
    const ITransmissionAnalogModel *analogModel = NULL;
    if (digitalAnalogConverter && sampleModel)
        analogModel = digitalAnalogConverter->convertDigitalToAnalog(sampleModel);
    else if (digitalAnalogConverter && !sampleModel)
        throw cRuntimeError("Digital/analog converters need sample representation");
    // TODO: analog model is obligatory
    analogModel = createAnalogModel(bitModel->getHeaderBitLength(), bitModel->getHeaderBitRate(), bitModel->getPayloadBitLength(), bitModel->getPayloadBitRate());
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
