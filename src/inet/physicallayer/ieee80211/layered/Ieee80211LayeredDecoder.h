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

#ifndef __INET_IEEE80211LAYEREDDECODER_H
#define __INET_IEEE80211LAYEREDDECODER_H

#include "inet/physicallayer/contract/ISerializer.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaver.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Scrambler.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaving.h"
#include "inet/physicallayer/common/ConvolutionalCoder.h"
#include "inet/physicallayer/ieee80211/Ieee80211OFDMCode.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/contract/ISignalPacketModel.h"
#include "inet/physicallayer/contract/ISignalBitModel.h"
#include "inet/physicallayer/contract/IDecoder.h"

namespace inet {
namespace physicallayer {

// TODO: rename to Ieee..OFDMDecoder
class INET_API Ieee80211LayeredDecoder : public IDecoder
{
    protected:
        const Ieee80211OFDMCode *code;
        const IScrambler *descrambler;
        const IFECCoder *fecDecoder;
        const IInterleaver *deinterleaver;
        Hz channelSpacing;

    protected:
        const IReceptionPacketModel *createPacketModel(const BitVector *decodedBits, const IScrambling *scrambling, const IForwardErrorCorrection *fec, const IInterleaving *interleaving) const;
        ShortBitVector getSignalFieldRate(const BitVector& signalField) const;
        unsigned int getSignalFieldLength(const BitVector& signalField) const;
        unsigned int calculatePadding(unsigned int dataFieldLengthInBits, const IModulation *modulationScheme, const Ieee80211ConvolutionalCode *fec) const;

    public:
        virtual void printToStream(std::ostream& stream) const { stream << "Ieee80211LayeredDecoder"; }
        const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const;
        const Ieee80211OFDMCode *getCode() const { return code; }
        Ieee80211LayeredDecoder(const Ieee80211OFDMCode *code);
        Ieee80211LayeredDecoder(const IScrambler *descrambler, const IFECCoder *fecDecoder, const IInterleaver *deinterleaver, Hz channelSpacing);
        virtual ~Ieee80211LayeredDecoder();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211LAYEREDDECODER_H */
