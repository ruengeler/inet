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

#ifndef __INET_IEEE80211INTERLEAVER_H
#define __INET_IEEE80211INTERLEAVER_H

#include "inet/common/INETDefs.h"
#include "inet/common/BitVector.h"
#include "inet/physicallayer/contract/IInterleaver.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaving.h"

namespace inet {
namespace physicallayer {

/*
 * It is a IEEE 802.11 block interleaver/deinterleaver implementation.
 * The permutation equations and all the details can be found in:
 * Part 11: Wireless LAN Medium Access Control (MAC) and Physical Layer (PHY) Specifications,
 * 18.3.5.7 Data interleaving
 */
class Ieee80211Interleaver : public InterleaverBase
{
  protected:
    int numberOfCodedBitsPerSymbol;
    int numberOfCodedBitsPerSubcarrier;
    int s;
    const Ieee80211Interleaving* interleaving;

  public:
    BitVector interleave(const BitVector& bits) const;
    BitVector deinterleave(const BitVector& bits) const;
    int getNumberOfCodedBitsPerSymbol() const { return numberOfCodedBitsPerSymbol; }
    int getNumberOfCodedBitsPerSubcarrier() const { return numberOfCodedBitsPerSubcarrier; }
    const Ieee80211Interleaving *getInterleaving() const { return interleaving; }
    Ieee80211Interleaver(const Ieee80211Interleaving *interleaving);
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211INTERLEAVER_H */
