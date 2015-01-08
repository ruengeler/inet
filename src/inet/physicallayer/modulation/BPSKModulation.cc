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

#include "inet/physicallayer/modulation/BPSKModulation.h"

namespace inet {

namespace physicallayer {

const double BPSKModulation::kMOD = 1;
const APSKSymbol BPSKModulation::encodingTable[] = {APSKSymbol(-1,0), APSKSymbol(1,0)};
const BPSKModulation BPSKModulation::singleton;

BPSKModulation::BPSKModulation() : APSKModulationBase(encodingTable, 1, 2, kMOD)
{
}

double BPSKModulation::calculateBER(double snir, double bandwidth, double bitrate) const
{
    // TODO: review 1/2*erfc(sqrt(snir))? according to http://www.dsplog.com/2007/08/05/bit-error-probability-for-bpsk-modulation/
    return 0.5 * exp(-snir * bandwidth / bitrate);
}

double BPSKModulation::calculateSER(double snir) const
{
    // http://www.dsplog.com/2012/01/01/symbol-error-rate-16qam-64qam-256qam/
    double c = erfc(kMOD * sqrt(snir));
    return 2 * (1 - 1.0 / sqrt(constellationSize)) * c - (1 - 2.0 / sqrt(constellationSize) + 1.0 / constellationSize) * c * c;
}

} // namespace physicallayer

} // namespace inet

