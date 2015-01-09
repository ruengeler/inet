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

#ifndef __INET_APSKMODULATIONBASE_H
#define __INET_APSKMODULATIONBASE_H

#include "inet/physicallayer/contract/IAPSKModulation.h"
#include "inet/common/ShortBitVector.h"
#include "inet/physicallayer/apsk/layered/APSKSymbol.h"

namespace inet {

namespace physicallayer {

/**
 * Base class for modulations using various amplitude and phase-shift keying.
 */
class INET_API APSKModulationBase : public IAPSKModulation
{
    protected:
        const std::vector<APSKSymbol> *constellation;
        const int codeWordSize;
        const int constellationSize;
        const double normalizationFactor;

    public:
        APSKModulationBase(const std::vector<APSKSymbol> *constellation, double normalizationFactor);

        virtual void printToStream(std::ostream &stream) const;
        const static APSKModulationBase *findModulation(const char *name);

        virtual const std::vector<APSKSymbol> *getConstellation() const { return constellation; }
        virtual int getConstellationSize() const { return constellationSize; }
        virtual int getCodeWordSize() const { return codeWordSize; }
        virtual double getNormalizationFactor() const { return normalizationFactor; }

        virtual const APSKSymbol *mapToConstellationDiagram(const ShortBitVector& symbol) const;
        virtual ShortBitVector demapToBitRepresentation(const APSKSymbol *symbol) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_APSKMODULATIONBASE_H

