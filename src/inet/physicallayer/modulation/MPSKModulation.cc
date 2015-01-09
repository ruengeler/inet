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

#include "inet/physicallayer/modulation/MPSKModulation.h"

namespace inet {

namespace physicallayer {

static std::vector<APSKSymbol> *createConstellation(unsigned int codeWordSize)
{
    auto symbols = new std::vector<APSKSymbol>();
    unsigned int constellationSize = pow(2, codeWordSize);
    for (unsigned int i = 0; i < constellationSize; i++) {
        // TODO: gray code?
        double alpha = 2 * M_PI * i / constellationSize;
        symbols->push_back(APSKSymbol(cos(alpha), sin(alpha)));
    }
    return symbols;
}

MPSKModulation::MPSKModulation(unsigned int codeWordSize) : APSKModulationBase(createConstellation(codeWordSize), 1)
{
}

MPSKModulation::~MPSKModulation()
{
    delete constellation;
}

void MPSKModulation::printToStream(std::ostream &stream) const
{
    stream << "MPSKModulaiton, ";
    APSKModulationBase::printToStream(stream);
}

double MPSKModulation::calculateBER(double snir, double bandwidth, double bitrate) const
{
    // http://www.dsplog.com/2008/05/18/bit-error-rate-for-16psk-modulation-using-gray-mapping/
    return erfc(sqrt(snir) * sin(M_PI / constellationSize)) / codeWordSize;
}

double MPSKModulation::calculateSER(double snir) const
{
    // http://www.dsplog.com/2008/03/18/symbol-error-rate-for-16psk/
    return erfc(sqrt(snir) * sin(M_PI / constellationSize));
}

} // namespace physicallayer

} // namespace inet

