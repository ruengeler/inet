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

#ifndef __INET_IEEE80211LAYEREDDECODER_H_
#define __INET_IEEE80211LAYEREDDECODER_H_

#include "inet/physicallayer/layered/LayeredDecoder.h"
#include "inet/physicallayer/contract/ISerializer.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaver.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Scrambler.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaving.h"
#include "inet/physicallayer/common/ConvolutionalCoder.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/base/APSKModulationBase.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211LayeredDecoder : public LayeredDecoder
{
    protected:
        const Ieee80211Scrambler *descrambler;
        const ConvolutionalCoder *fecDecoder;
        const Ieee80211Interleaver *deinterleaver;

    protected:
        const IReceptionPacketModel *createPacketModel(const BitVector& decodedBits, const Ieee80211Scrambling *scrambling, const ConvolutionalCode *fec, const Ieee80211Interleaving *interleaving) const;
        const Ieee80211ConvolutionalCode *getFecFromSignalFieldRate(const ShortBitVector& rate) const;
        const APSKModulationBase *getModulationFromSignalFieldRate(const ShortBitVector& rate) const;
        const Ieee80211Interleaving *getInterleavingFromModulation(const IModulation *modulationScheme) const;
        ShortBitVector getSignalFieldRate(const BitVector& signalField) const;
        unsigned int getSignalFieldLength(const BitVector& signalField) const;
        unsigned int calculatePadding(unsigned int dataFieldLengthInBits, const IModulation *modulationScheme, const Ieee80211ConvolutionalCode *fec) const;

    public:
        virtual void printToStream(std::ostream& stream) const { stream << "Ieee80211LayeredDecoder"; }
        const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const;
        Ieee80211LayeredDecoder(const Ieee80211Scrambler *descrambler, const ConvolutionalCoder *fecDecoder, const Ieee80211Interleaver *deinterleaver);
        virtual ~Ieee80211LayeredDecoder();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211LAYEREDDECODER_H_ */
