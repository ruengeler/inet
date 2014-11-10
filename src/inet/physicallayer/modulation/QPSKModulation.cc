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

#include "inet/physicallayer/modulation/QPSKModulation.h"

namespace inet {
namespace physicallayer {

const double QPSKModulation::kMOD = 1 / sqrt(2);
const APSKSymbol QPSKModulation::encodingTable[] = {kMOD * APSKSymbol(-1,-1), kMOD * APSKSymbol(1,-1), kMOD * APSKSymbol(-1, 1), kMOD * APSKSymbol(1,1)};
const QPSKModulation QPSKModulation::singleton;

QPSKModulation::QPSKModulation() : APSKModulationBase(encodingTable, 2, 4, kMOD)
{
}

double QPSKModulation::calculateBER(double snir, double bandwidth, double bitrate) const
{
    throw cRuntimeError("Unimplemented");
}

double QPSKModulation::calculateSER(double snir) const
{
    double c = erfc(kMOD * sqrt(snir));
    return 2 * (1 - 1 / sqrt(4)) * c - (1 - 2 / sqrt(4) + 1 / 4) * c * c;
}

} /* namespace physicallayer */
} /* namespace inet */
