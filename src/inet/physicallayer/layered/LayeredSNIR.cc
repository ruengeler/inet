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

#include "inet/physicallayer/contract/ISNIR.h"
#include "inet/physicallayer/layered/LayeredSNIR.h"

namespace inet {
namespace physicallayer {

LayeredSNIR::LayeredSNIR(const LayeredReception *reception, const ScalarNoise *noise) :
        SNIRBase(reception, noise)
{

}

double LayeredSNIR::getMin() const
{
    if (isNaN(minSNIR))
        minSNIR = computeMin();
    return minSNIR;
}

double LayeredSNIR::computeMin() const
{
    const LayeredReception *layeredReception = check_and_cast<const LayeredReception *>(reception);
    const ScalarNoise *scalarNoise= check_and_cast<const ScalarNoise *>(noise);
    return unit(layeredReception->getPower() / scalarNoise->computeMaxPower(reception->getStartTime(), reception->getEndTime())).get();
}

LayeredSNIR::~LayeredSNIR()
{
}

void LayeredSNIR::printToStream(std::ostream& stream) const
{
    stream << "Layered SNIR";
}

} /* namespace physicallayer */
} /* namespace inet */

