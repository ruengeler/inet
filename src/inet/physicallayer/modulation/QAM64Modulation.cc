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

#include "inet/physicallayer/modulation/QAM64Modulation.h"

namespace inet {

namespace physicallayer {

const double QAM64Modulation::kMOD = 1 / sqrt(42);
const APSKSymbol QAM64Modulation::encodingTable[] = {kMOD * APSKSymbol(-7, -7), kMOD * APSKSymbol(7, -7), kMOD * APSKSymbol(-1, -7), kMOD * APSKSymbol(1, -7), kMOD * APSKSymbol(-5, -7),
                                                     kMOD * APSKSymbol(5, -7), kMOD * APSKSymbol(-3, -7), kMOD * APSKSymbol(3, -7), kMOD * APSKSymbol(-7, 7), kMOD * APSKSymbol(7, 7),
                                                     kMOD * APSKSymbol(-1, 7), kMOD * APSKSymbol(1, 7), kMOD * APSKSymbol(-5, 7), kMOD * APSKSymbol(5, 7), kMOD * APSKSymbol(-3, 7),
                                                     kMOD * APSKSymbol(3, 7), kMOD * APSKSymbol(-7, -1), kMOD * APSKSymbol(7, -1), kMOD * APSKSymbol(-1, -1), kMOD * APSKSymbol(1, -1),
                                                     kMOD * APSKSymbol(-5, -1), kMOD * APSKSymbol(5, -1), kMOD * APSKSymbol(-3, -1), kMOD * APSKSymbol(3, -1), kMOD * APSKSymbol(-7, 1),
                                                     kMOD * APSKSymbol(7, 1), kMOD * APSKSymbol(-1, 1), kMOD * APSKSymbol(1, 1), kMOD * APSKSymbol(-5, 1), kMOD * APSKSymbol(5, 1),
                                                     kMOD * APSKSymbol(-3, 1), kMOD * APSKSymbol(3, 1), kMOD * APSKSymbol(-7, -5), kMOD * APSKSymbol(7, -5), kMOD * APSKSymbol(-1, -5),
                                                     kMOD * APSKSymbol(1, -5), kMOD * APSKSymbol(-5, -5), kMOD * APSKSymbol(5, -5), kMOD * APSKSymbol(-3, -5), kMOD * APSKSymbol(3, -5),
                                                     kMOD * APSKSymbol(-7, 5), kMOD * APSKSymbol(7, 5), kMOD * APSKSymbol(-1, 5), kMOD * APSKSymbol(1, 5), kMOD * APSKSymbol(-5, 5),
                                                     kMOD * APSKSymbol(5, 5), kMOD * APSKSymbol(-3, 5), kMOD * APSKSymbol(3, 5), kMOD * APSKSymbol(-7, -3), kMOD * APSKSymbol(7, -3),
                                                     kMOD * APSKSymbol(-1, -3), kMOD * APSKSymbol(1, -3), kMOD * APSKSymbol(-5, -3), kMOD * APSKSymbol(5, -3), kMOD * APSKSymbol(-3, -3),
                                                     kMOD * APSKSymbol(3, -3), kMOD * APSKSymbol(-7, 3), kMOD * APSKSymbol(7, 3), kMOD * APSKSymbol(-1, 3), kMOD * APSKSymbol(1, 3),
                                                     kMOD * APSKSymbol(-5, 3), kMOD * APSKSymbol(5, 3), kMOD * APSKSymbol(-3, 3), kMOD * APSKSymbol(3, 3)};
const QAM64Modulation QAM64Modulation::singleton;

QAM64Modulation::QAM64Modulation() : APSKModulationBase(encodingTable, 6, 64, kMOD)
{
}

double QAM64Modulation::calculateBER(double snir, double bandwidth, double bitrate) const
{
    // TODO:
    throw cRuntimeError("Unimplemented.");
}

double QAM64Modulation::calculateSER(double snir) const
{
    // http://www.dsplog.com/2012/01/01/symbol-error-rate-16qam-64qam-256qam/
    double c = erfc(kMOD * sqrt(snir));
    return 2 * (1 - 1.0 / sqrt(constellationSize)) * c - (1 - 2.0 / sqrt(constellationSize) + 1.0 / constellationSize) * c * c;
}

} // namespace physicallayer

} // namespace inet

