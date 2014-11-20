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

#include "Ieee80211OFDMModulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"

namespace inet {
namespace physicallayer {

const IAPSKModulation* Ieee80211OFDMModulation::getModulation(uint8_t rate) const
{
    // Table 18-6—Contents of the SIGNAL field
    // Table 18-4—Modulation-dependent parameters
    if (rate == 1101_b || rate == 1111_b)
        return &BPSKModulation::singleton;
    else if (rate == 0101_b || rate == 0111_b)
        return &QPSKModulation::singleton;
    else if (rate == 1001_b || rate == 1011_b)
        return &QAM16Modulation::singleton;
    else if(rate == 0001_b || rate == 0011_b)
        return &QAM64Modulation::singleton;
    else
        throw cRuntimeError("Unknown rate field = %d", rate);
}

bps Ieee80211OFDMModulation::computeHeaderBitrate(Hz channelSpacing) const
{
    // TODO: Revise, these are the minimum bitrates for each channel spacing.
    if (channelSpacing == MHz(20))
        return bps(6000000);
    else if (channelSpacing == MHz(10))
        return bps(3000000);
    else if (channelSpacing == MHz(5))
        return bps(1500000);
    else
        throw cRuntimeError("Invalid channel spacing %lf", channelSpacing.get());
}

bps Ieee80211OFDMModulation::computeDataBitrate(uint8_t rate, Hz channelSpacing) const
{
    double rateFactor;
    if (channelSpacing == MHz(20))
        rateFactor = 1;
    else if (channelSpacing == MHz(10))
        rateFactor = 0.5;
    else if (channelSpacing == MHz(5))
        rateFactor = 0.25;
    else
        throw cRuntimeError("Unknown channel spacing = %lf", channelSpacing);
    if (rate == 1101_b)
        return bps(6000000 * rateFactor);
    else if (rate == 1111_b)
        return bps(9000000 * rateFactor);
    else if (rate == 0101_b)
        return bps(12000000 * rateFactor);
    else if (rate == 0111_b)
        return bps(18000000 * rateFactor);
    else if (rate == 1001_b)
        return bps(24000000 * rateFactor);
    else if (rate == 1011_b)
        return bps(36000000 * rateFactor);
    else if (rate == 0001_b)
        return bps(48000000 * rateFactor);
    else if (rate == 0011_b)
        return bps(54000000 * rateFactor);
    else
        throw cRuntimeError("Invalid rate field: %d", rate);
}

Ieee80211OFDMModulation::Ieee80211OFDMModulation(uint8_t signalRateField, Hz channelSpacing) :
        signalRateField(signalRateField),
        channelSpacing(channelSpacing)
{
    modulationScheme = getModulation(signalRateField);
    headerRate = computeHeaderBitrate(channelSpacing);
    dataRate = computeDataBitrate(signalRateField, channelSpacing);
}

Ieee80211OFDMModulation::~Ieee80211OFDMModulation()
{
}

} /* namespace physicallayer */
} /* namespace inet */
