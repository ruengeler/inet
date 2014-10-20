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

#ifndef __INET_LAYEREDATTENUATION_H
#define __INET_LAYEREDATTENUATION_H

#include "inet/physicallayer/base/AttenuationBase.h"
#include "inet/physicallayer/layered/SignalAnalogModel.h"

namespace inet {
namespace physicallayer {

// TODO: revise name: LayeredScalarAttenuation would be better
class INET_API LayeredAttenuation : public AttenuationBase
{
    protected:
        virtual const ScalarReceptionAnalogModel *computeReceptionAnalogModel(const ScalarTransmissionAnalogModel *analogModel) const;

    public:
      virtual void printToStream(std::ostream& stream) const { stream << "Layered attenuation"; }
      virtual const IReception *computeReception(const IRadio *receiverRadio, const ITransmission *transmission) const;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_LAYEREDATTENUATION_H */
