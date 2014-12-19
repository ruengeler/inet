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

#ifndef __INET_APSKMODULATOR_H
#define __INET_APSKMODULATOR_H

#include "inet/physicallayer/contract/layered/IModulator.h"
#include "inet/physicallayer/layered/SignalBitModel.h"
#include "inet/physicallayer/layered/SignalSymbolModel.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/apsk/layered/APSKSymbol.h"

namespace inet {

namespace physicallayer {

class INET_API APSKModulator : public IModulator
{
  protected:
    const APSKModulationBase *modulationScheme;
    static const int polarityVector[127];

  public:
    virtual const ITransmissionSymbolModel *modulate(const ITransmissionBitModel *bitModel) const;
    const IModulation *getModulationScheme() const { return modulationScheme; }
    void printToStream(std::ostream& stream) const { stream << "APSKModulator"; }
    APSKModulator(const APSKModulationBase *modulationScheme);
    ~APSKModulator();
};

} // namespace physicallayer
} // namespace inet

#endif /* __INET_APSKMODULATOR_H */
