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

#ifndef __INET_LAYEREDSCALARTRANSMISSION_H
#define __INET_LAYEREDSCALARTRANSMISSION_H

#include "inet/physicallayer/layered/LayeredTransmission.h"

namespace inet {
namespace physicallayer {

class INET_API LayeredScalarTransmission : public LayeredTransmission
{
  protected:
    const Hz carrierFrequency;
    const W power;

  public:
    const Hz getCarrierFrequency() const { return carrierFrequency; }
    const W getPower() const { return power; }
    LayeredScalarTransmission(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel, const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, Hz bandwidth, Hz carrierFrequency, W power);
};

} // namespace physicallayer
} // namespace inet

#endif // __INET_LAYEREDSCALARTRANSMISSION_H
