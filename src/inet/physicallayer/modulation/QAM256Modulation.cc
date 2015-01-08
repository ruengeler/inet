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

#include "inet/physicallayer/modulation/QAM256Modulation.h"

namespace inet {

namespace physicallayer {

const double QAM256Modulation::kMOD = 1 / sqrt(170);
// TODO: fill in
const APSKSymbol QAM256Modulation::encodingTable[] = {};
const QAM256Modulation QAM256Modulation::singleton;

QAM256Modulation::QAM256Modulation() : APSKModulationBase(encodingTable, 8, 256, kMOD)
{
}

double QAM256Modulation::calculateBER(double snir, double bandwidth, double bitrate) const
{
    return 0.25 * (1 - 1 / sqrt(pow(2.0, 8))) * erfc(snir * bandwidth / bitrate);
}

double QAM256Modulation::calculateSER(double snir) const
{
    // http://www.dsplog.com/2012/01/01/symbol-error-rate-16qam-64qam-256qam/
    double c = erfc(kMOD * sqrt(snir));
    return 2 * (1 - 1.0 / sqrt(constellationSize)) * c - (1 - 2.0 / sqrt(constellationSize) + 1.0 / constellationSize) * c * c;
}

} // namespace physicallayer

} // namespace inet

