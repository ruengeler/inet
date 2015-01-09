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

#include "inet/physicallayer/modulation/QAM16Modulation.h"

namespace inet {

namespace physicallayer {

const std::vector<APSKSymbol> QAM16Modulation::constellation = {APSKSymbol(-3, -3), APSKSymbol(3, -3), APSKSymbol(-1, -3),
                                                                APSKSymbol(1, -3), APSKSymbol(-3, 3), APSKSymbol(3, 3),
                                                                APSKSymbol(-1, 3), APSKSymbol(1, 3), APSKSymbol(-3, -1),
                                                                APSKSymbol(3, -1), APSKSymbol(-1, -1), APSKSymbol(1, -1),
                                                                APSKSymbol(-3, 1), APSKSymbol(3, 1), APSKSymbol(-1, 1),
                                                                APSKSymbol(1, 1)};

const QAM16Modulation QAM16Modulation::singleton;

QAM16Modulation::QAM16Modulation() : MQAMModulationBase(&constellation, 1 / sqrt(10))
{
}

double QAM16Modulation::calculateBER(double snir, double bandwidth, double bitrate) const
{
    return 0.5 * (1 - 1 / sqrt(pow(2.0, 4))) * erfc(snir * bandwidth / bitrate);
}

} // namespace physicallayer

} // namespace inet

