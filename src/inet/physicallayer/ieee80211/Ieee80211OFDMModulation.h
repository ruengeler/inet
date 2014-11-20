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

#ifndef __INET_IEEE80211OFDMMODULATION_H
#define __INET_IEEE80211OFDMMODULATION_H

#include "inet/physicallayer/contract/IModulation.h"
#include "inet/physicallayer/contract/IAPSKModulation.h"
#include "inet/common/Units.h"
#include "inet/common/INETUtils.h"

namespace inet {
namespace physicallayer {

using namespace units::values;
using namespace utils;

class INET_API Ieee80211OFDMModulation
{
    protected:
        const IAPSKModulation *modulationScheme;
        uint8_t signalRateField;
        Hz channelSpacing;
        bps headerRate;
        bps dataRate;

    protected:
        const IAPSKModulation* getModulation(uint8_t rate) const;
        bps computeDataBitrate(uint8_t rate, Hz channelSpacing) const;
        bps computeHeaderBitrate(Hz channelSpacing) const;

    public:
        virtual double calculateBER(double snir, double bandwidth, double bitrate) { return modulationScheme->calculateBER(snir, bandwidth, bitrate); }
        virtual double calculateSER(double snir) const { return modulationScheme->calculateSER(snir); }
        virtual Hz getChannelSpacing() const { return channelSpacing; }
        const bps getDataRate() const { return dataRate; }
        const bps getHeaderRate() const { return headerRate; }
        uint8_t getSignalRateField() const { return signalRateField; }
        const IAPSKModulation* getModulationScheme() const { return modulationScheme; }
        Ieee80211OFDMModulation(uint8_t signalRateField, Hz channelSpacing);
        virtual ~Ieee80211OFDMModulation();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211OFDMMODULATION_H */
