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

#ifndef __INET_LAYEREDRECEPTION_H
#define __INET_LAYEREDRECEPTION_H

#include "inet/physicallayer/contract/ISignalPacketModel.h"
#include "inet/physicallayer/contract/ISignalBitModel.h"
#include "inet/physicallayer/contract/ISignalSymbolModel.h"
#include "inet/physicallayer/contract/ISignalSampleModel.h"
#include "inet/physicallayer/contract/ISignalAnalogModel.h"
#include "inet/physicallayer/analogmodel/ScalarReception.h"

namespace inet {

namespace physicallayer {
// TODO: do not inherit from ScalarReception
class INET_API LayeredScalarReception : public ScalarReception
{
  protected:
    const IReceptionAnalogModel *analogModel;

  public:
    LayeredScalarReception(const IReceptionAnalogModel *analogModel, const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, const Hz carrierFrequency, const Hz bandwidth, const W power) :
        ScalarReception(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, carrierFrequency, bandwidth, power),
        analogModel(analogModel)
    {}

    virtual const IReceptionAnalogModel *getAnalogModel() const { return analogModel; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_LAYEREDRECEPTION_H
