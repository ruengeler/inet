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

#ifndef __INET_IEEE80211LAYEREDENCODER_H
#define __INET_IEEE80211LAYEREDENCODER_H

#include "inet/physicallayer/contract/IEncoder.h"
#include "inet/physicallayer/contract/ISerializer.h"
#include "inet/physicallayer/contract/IFECCoder.h"
#include "inet/physicallayer/contract/IScrambler.h"
#include "inet/physicallayer/contract/IInterleaver.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/layered/SignalBitModel.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/ieee80211/Ieee80211OFDMCode.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211LayeredEncoder : public IEncoder
{
    protected:
        const IFECCoder *fecEncoder;
        const IInterleaver *interleaver;
        const IScrambler *scrambler;
        const Ieee80211OFDMCode *code;
        Hz channelSpacing;

    protected:
        uint8_t getRate(const BitVector *serializedPacket) const;

    public:
        virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const;
        virtual void printToStream(std::ostream& stream) const { stream << "IEEE80211 Layered Encoder"; } // TODO
        const Ieee80211OFDMCode *getCode() const { return code; }
        Ieee80211LayeredEncoder();
        Ieee80211LayeredEncoder(const Ieee80211OFDMCode *code);
        Ieee80211LayeredEncoder(const IFECCoder *fecEncoder, const IInterleaver *interleaver, const IScrambler *scrambler, Hz channelSpacing);
        ~Ieee80211LayeredEncoder();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211LAYEREDENCODER_H */
