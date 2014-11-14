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

#ifndef __INET_LAYEREDSCALARANALOGMODEL_H
#define __INET_LAYEREDSCALARANALOGMODEL_H

#include "inet/physicallayer/base/AnalogModelBase.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"
#include "inet/physicallayer/layered/LayeredScalarReception.h"

namespace inet {

namespace physicallayer {

class INET_API LayeredScalarAnalogModel : public AnalogModelBase
{
  protected:
    virtual bool areOverlappingBands(Hz carrierFrequency1, Hz bandwidth1, Hz carrierFrequency2, Hz bandwidth2) const;
    const ScalarReceptionSignalAnalogModel* computeReceptionAnalogModel(const ScalarTransmissionSignalAnalogModel* analogModel) const;

  public:
    virtual void printToStream(std::ostream& stream) const { stream << "LayeredScalarAnalogModel"; }

    virtual const IReception *computeReception(const IRadio *radio, const ITransmission *transmission) const;
    virtual const INoise *computeNoise(const IListening *listening, const IInterference *interference) const;
    virtual const ISNIR *computeSNIR(const IReception *reception, const INoise *noise) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_LAYEREDSCALARANALOGMODEL_H

