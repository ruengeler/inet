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

#include "inet/physicallayer/analogmodel/layered/LayeredSNIR.h"
#include "inet/physicallayer/analogmodel/layered/LayeredScalarAnalogModel.h"

namespace inet {

namespace physicallayer {

LayeredSNIR::LayeredSNIR(const LayeredReception *reception, const ScalarNoise *noise) :
    SNIRBase(reception, noise),
    minSNIR(NaN)
{
}

void LayeredSNIR::printToStream(std::ostream& stream) const
{
    stream << "LayeredSNIR, "
           << "minSNIR = " << minSNIR;
}

double LayeredSNIR::computeMin() const
{
    const ScalarNoise *scalarNoise = check_and_cast<const ScalarNoise *>(noise);
    const LayeredReception *layeredReception = dynamic_cast<const LayeredReception *>(reception);
    // TODO: scalar?
    const ScalarReceptionSignalAnalogModel *analogModel = dynamic_cast<const ScalarReceptionSignalAnalogModel*>(layeredReception->getAnalogModel());
    return unit(analogModel->getPower() / scalarNoise->computeMaxPower(layeredReception->getStartTime(), layeredReception->getEndTime())).get();
}

double LayeredSNIR::getMin() const
{
    if (isNaN(minSNIR))
        minSNIR = computeMin();
    return minSNIR;
}

} // namespace physicallayer

} // namespace inet

