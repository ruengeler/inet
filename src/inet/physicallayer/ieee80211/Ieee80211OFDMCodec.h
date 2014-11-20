//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_IEEE80211OFDMCODEC_H
#define __INET_IEEE80211OFDMCODEC_H

#include "inet/common/INETDefs.h"
#include "inet/common/Units.h"
#include "inet/physicallayer/ieee80211/Ieee80211OFDMModulation.h"
#include "inet/physicallayer/contract/IModulation.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Scrambling.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaving.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211ConvolutionalCode.h"

namespace inet {
namespace physicallayer {

using namespace units::values;

class INET_API Ieee80211OFDMCodec
{
    protected:
        uint8_t signalFieldRate;
        Hz channelSpacing;
        const Ieee80211ConvolutionalCode *convCode;
        const Ieee80211Interleaving *interleaving;
        const Ieee80211Scrambling *scrambling;

    protected:
        const Ieee80211ConvolutionalCode *computeFec(uint8_t rate) const;
        const Ieee80211Interleaving *computeInterleaving(const IModulation *modulationScheme) const;
        const Ieee80211Scrambling *computeScrambling() const;

    public:
        const Ieee80211ConvolutionalCode *getConvCode() const { return convCode; }
        const Ieee80211Interleaving *getInterleaving() const { return interleaving; }
        const Ieee80211Scrambling *getScrambling() const { return scrambling; }
        uint8_t getSignalFieldRate() const { return signalFieldRate; }

        Ieee80211OFDMCodec(uint8_t signalFieldRate, Hz channelSpacing);
        virtual ~Ieee80211OFDMCodec();

};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211OFDMCODEC_H */
