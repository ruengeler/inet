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

#include "inet/physicallayer/base/MQAMModulationBase.h"

namespace inet {

namespace physicallayer {

MQAMModulationBase::MQAMModulationBase(const std::vector<APSKSymbol> *constellation, double normalizationFactor) :
        APSKModulationBase(constellation, normalizationFactor)
{
}

double MQAMModulationBase::calculateSER(double snir) const
{
    // http://www.dsplog.com/2012/01/01/symbol-error-rate-16qam-64qam-256qam/
    // http://en.wikipedia.org/wiki/Eb/N0
    double bandwidth = 0; // TODO:
    double bitrate = 0; // TODO:
    double EbN0 = snir * bandwidth / bitrate;
    double EsN0 = EbN0 * log2(constellationSize);
    double c = erfc(normalizationFactor * sqrt(EsN0));
    return 2 * (1 - 1.0 / sqrt(constellationSize)) * c - (1 - 2.0 / sqrt(constellationSize) + 1.0 / constellationSize) * c * c;
}

} // namespace physicallayer

} // namespace inet

