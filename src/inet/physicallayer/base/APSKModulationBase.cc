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

#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/QAM256Modulation.h"
#include "inet/common/Complex.h"

namespace inet {

namespace physicallayer {

APSKModulationBase::APSKModulationBase(const APSKSymbol *encodingTable, int codeWordLength, int constellationSize, double normalizationFactor) :
    encodingTable(encodingTable),
    codeWordLength(codeWordLength),
    constellationSize(constellationSize),
    normalizationFactor(normalizationFactor)
{
}

void APSKModulationBase::printToStream(std::ostream &stream) const
{
    stream << "APSK modulation with";
    stream << " constellation size = " << constellationSize << " codeword length = " << codeWordLength;
    stream << " normalization factor = " << normalizationFactor << endl;
}

const APSKModulationBase *APSKModulationBase::findModulation(const char *modulation)
{
    if (!strcmp("BPSK", modulation))
        return &BPSKModulation::singleton;
    else if (!strcmp("QPSK", modulation))
        return &QPSKModulation::singleton;
    else if (!strcmp("QAM-16", modulation))
        return &QAM16Modulation::singleton;
    else if (!strcmp("QAM-64", modulation))
        return &QAM64Modulation::singleton;
    else if (!strcmp("QAM-256", modulation))
        return &QAM256Modulation::singleton;
    else
        throw cRuntimeError("Unknown modulation = %s", modulation);
}

const APSKSymbol *APSKModulationBase::mapToConstellationDiagram(const ShortBitVector& symbol) const
{
    int decimalSymbol = symbol.toDecimal();
    if (decimalSymbol >= constellationSize)
        throw cRuntimeError("Unknown input: %d", decimalSymbol);
    return &encodingTable[decimalSymbol];
}

ShortBitVector APSKModulationBase::demapToBitRepresentation(const APSKSymbol* symbol) const
{
    // TODO: Complete implementation: http://eprints.soton.ac.uk/354719/1/tvt-hanzo-2272640-proof.pdf
    double symbolQ = symbol->getReal();
    double symbolI = symbol->getImaginary();
    double minDist = DBL_MAX;
    int nearestNeighborIndex = -1;
    for (int i = 0; i < constellationSize; i++)
    {
        const APSKSymbol *constellationSymbol = &encodingTable[i];
        double cQ = constellationSymbol->getReal();
        double cI = constellationSymbol->getImaginary();
        double dist = (symbolQ - cQ) * (symbolQ - cQ) + (symbolI - cI) * (symbolI - cI);
        if (dist < minDist)
        {
            minDist = dist;
            nearestNeighborIndex = i;
        }
    }
    ASSERT(nearestNeighborIndex != -1);
    return ShortBitVector(nearestNeighborIndex, codeWordLength);
}

} // namespace physicallayer

} // namespace inet

