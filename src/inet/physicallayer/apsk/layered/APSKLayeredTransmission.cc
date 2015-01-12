//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "inet/physicallayer/apsk/layered/APSKLayeredTransmission.h"

namespace inet {
namespace physicallayer {

APSKLayeredTransmission::APSKLayeredTransmission(
        const ITransmissionPacketModel* packetModel,
        const ITransmissionBitModel* bitModel,
        const ITransmissionSymbolModel* symbolModel,
        const ITransmissionSampleModel* sampleModel,
        const ITransmissionAnalogModel* analogModel, const IRadio* transmitter,
        const cPacket* macFrame, const IModulation* modulation,
        const simtime_t startTime, const simtime_t endTime,
        const Coord startPosition, const Coord endPosition,
        const EulerAngles startOrientation, const EulerAngles endOrientation) :
                LayeredTransmission(packetModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
                modulation(modulation)
{

}

} /* namespace physicallayer */
} /* namespace inet */

