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

#ifndef __INET_IEEE80211LAYEREDENCODERMODULE_H
#define __INET_IEEE80211LAYEREDENCODERMODULE_H

#include "inet/physicallayer/ieee80211/layered/Ieee80211LayeredEncoder.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211LayeredEncoderModule : public IEncoder, public cSimpleModule
{
    protected:
        const Ieee80211LayeredEncoder *encoder;
        const ISerializer *serializer;
        const IScrambler *scrambler;
        const IFECCoder *fecEncoder;
        const IInterleaver *interleaver;
        Hz channelSpacing;
        bps headerBitrate;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }

    public:
        virtual void printToStream(std::ostream& stream) const { stream << "Ieee80211LayeredEncoder"; }
        virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const;
        virtual ~Ieee80211LayeredEncoderModule();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211LAYEREDENCODERMODULE_H */
