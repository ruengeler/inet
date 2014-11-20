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

#ifndef __INET_IEEE80211LAYEREDRECEIVER_H
#define __INET_IEEE80211LAYEREDRECEIVER_H

#include "inet/physicallayer/layered/LayeredReceiver.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/contract/IRadioMedium.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211LayeredReceiver : public LayeredReceiver
{
    protected:
        Hz channelSpacing;

    protected:
        virtual void initialize(int stage);
        const IReceptionSymbolModel *createSignalFieldReceptionSymbolModel(const IReceptionSymbolModel *receptionSymbolModel) const;
        const IReceptionPacketModel *demodulateAndDecodeSignalField(const IRadioMedium *medium, const IRadio *receiver, const LayeredScalarTransmission *transmission, const IReceptionSymbolModel *receptionSymbolModel) const;

    public:
        virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* IEEE80211LAYEREDRECEIVER_H_ */
