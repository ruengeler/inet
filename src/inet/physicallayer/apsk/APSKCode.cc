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

#include "APSKCode.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"

namespace inet {
namespace physicallayer {

#define SYMBOL_SIZE 48

const Ieee80211ConvolutionalCode* APSKCode::computeFec() const
{
    return new Ieee80211ConvolutionalCode(1, 2);
}

const Ieee80211Interleaving* APSKCode::computeInterleaving(const IModulation *modulationScheme) const
{
    const IAPSKModulation *dataModulationScheme = dynamic_cast<const IAPSKModulation*>(modulationScheme);
    ASSERT(dataModulationScheme != NULL);
    return new Ieee80211Interleaving(dataModulationScheme->getCodeWordLength() * SYMBOL_SIZE, dataModulationScheme->getCodeWordLength()); // FIXME: memory leak
}

const Ieee80211Scrambling* APSKCode::computeScrambling() const
{
    // Default scrambling
    return new Ieee80211Scrambling("1011101", "0001001");
}

APSKCode::APSKCode() :
    scrambling(NULL)
{
    convCode = new Ieee80211ConvolutionalCode(1,2);
    interleaving = new Ieee80211Interleaving(SYMBOL_SIZE, 1);
}

APSKCode::APSKCode(const ConvolutionalCode* convCode, const Ieee80211Interleaving* interleaving, const Ieee80211Scrambling* scrambling) :
        convCode(convCode),
        interleaving(interleaving),
        scrambling(scrambling)
{
}

APSKCode::~APSKCode()
{
    delete convCode;
    delete interleaving;
    delete scrambling;
}

} /* namespace physicallayer */
} /* namespace inet */

