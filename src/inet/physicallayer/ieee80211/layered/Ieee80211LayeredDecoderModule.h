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

#ifndef __INET_IEEE80211LAYEREDDECODERMODULE_H
#define __INET_IEEE80211LAYEREDDECODERMODULE_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/contract/IDecoder.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211LayeredDecoder.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaver.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Scrambler.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaving.h"
#include "inet/physicallayer/common/ConvolutionalCoder.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211LayeredDecoderModule : public cSimpleModule, public IDecoder
{
    protected:
        const Ieee80211LayeredDecoder *layeredDecoder;
        const IScrambler *descrambler;
        const IFECCoder *fecDecoder;
        const IInterleaver *deinterleaver;
        Hz channelSpacing;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }

    public:
        virtual void printToStream(std::ostream& stream) const { stream << "Ieee80211LayeredDecoder"; }
        const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const;
        virtual ~Ieee80211LayeredDecoderModule();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211LAYEREDDECODERMODULE_H */
