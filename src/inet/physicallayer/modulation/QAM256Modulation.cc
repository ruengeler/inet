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

const std::vector<APSKSymbol> QAM256Modulation::constellation = {}; // TODO:

const QAM256Modulation QAM256Modulation::singleton;

QAM256Modulation::QAM256Modulation() : MQAMModulationBase(&constellation, 1 / sqrt(170))
{
//    throw cRuntimeError("Not implemented");
}


} // namespace physicallayer

} // namespace inet

