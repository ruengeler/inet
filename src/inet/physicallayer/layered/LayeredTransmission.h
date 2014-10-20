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

#ifndef __INET_LAYEREDTRANSMISSION_H
#define __INET_LAYEREDTRANSMISSION_H

#include "inet/physicallayer/contract/ISignalPacketModel.h"
#include "inet/physicallayer/contract/ISignalBitModel.h"
#include "inet/physicallayer/contract/ISignalSymbolModel.h"
#include "inet/physicallayer/contract/ISignalSampleModel.h"
#include "inet/physicallayer/contract/ISignalAnalogModel.h"
#include "inet/physicallayer/base/TransmissionBase.h"

namespace inet {

namespace physicallayer {

// revise name: LayeredScalarTransmission would be better
class INET_API LayeredTransmission : public TransmissionBase
{
  protected:
    const ITransmissionPacketModel *packetModel;
    const ITransmissionBitModel    *bitModel;
    const ITransmissionSymbolModel *symbolModel;
    const ITransmissionSampleModel *sampleModel;
    const ITransmissionAnalogModel *analogModel;

    const Hz bandwidth;
    const Hz carrierFrequency;
    const W power;

  public:
    LayeredTransmission(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel, const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, Hz bandwidth, Hz carrierFreequency, W power) :
        TransmissionBase(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
        packetModel(packetModel),
        bitModel(bitModel),
        symbolModel(symbolModel),
        sampleModel(sampleModel),
        analogModel(analogModel),
        bandwidth(bandwidth),
        carrierFrequency(carrierFreequency),
        power(power)
    {}

    virtual const ITransmissionPacketModel *getPacketModel() const { return packetModel; }
    virtual const ITransmissionBitModel    *getBitModel()    const { return bitModel; }
    virtual const ITransmissionSymbolModel *getSymbolModel() const { return symbolModel; }
    virtual const ITransmissionSampleModel *getSampleModel() const { return sampleModel; }
    virtual const ITransmissionAnalogModel *getAnalogModel() const { return analogModel; }

    const Hz getBandwidth() const { return bandwidth; }
    const Hz getCarrierFrequency() const { return carrierFrequency; }
    const W getPower() const { return power; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_LAYEREDTRANSMISSION_H
