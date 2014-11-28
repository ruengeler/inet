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

#ifndef IEEE80211INTERLEAVERMODULE_H
#define IEEE80211INTERLEAVERMODULE_H

#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaver.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211InterleaverModule : public cSimpleModule, public IInterleaver
{
    protected:
        const Ieee80211Interleaver *interleaver;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { cRuntimeError("This module doesn't handle self messages."); }

    public:
        virtual BitVector interleave(const BitVector& bits) const { return interleaver->interleave(bits); }
        virtual BitVector deinterleave(const BitVector& bits) const { return interleaver->deinterleave(bits); }
        virtual const IInterleaving *getInterleaving() const { return interleaver->getInterleaving(); }
        virtual ~Ieee80211InterleaverModule();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* IEEE80211INTERLEAVERMODULE_H */
