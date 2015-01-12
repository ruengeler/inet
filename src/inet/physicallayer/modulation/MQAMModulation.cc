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

#include "inet/physicallayer/modulation/MQAMModulation.h"

namespace inet {

namespace physicallayer {

static std::vector<APSKSymbol> *createConstellation(unsigned int codeWordSize)
{
    auto symbols = new std::vector<APSKSymbol>();
    unsigned int constellationSize = pow(2, codeWordSize);
    for (unsigned int i = 0; i < constellationSize; i++) {
        throw cRuntimeError("Not implemented");
        // TODO: symbols->push_back(APSKSymbol(cos(alpha), sin(alpha)));
    }
    return symbols;
}

MQAMModulation::MQAMModulation(int codeWordSize) : MQAMModulationBase(createConstellation(codeWordSize), 1 / sqrt(2 * (pow(2, codeWordSize) - 1) / 3))
{
}

MQAMModulation::~MQAMModulation()
{
    delete constellation;
}

void MQAMModulation::printToStream(std::ostream &stream) const
{
    stream << "MQAMModulation, ";
    APSKModulationBase::printToStream(stream);
}

} // namespace physicallayer

} // namespace inet

