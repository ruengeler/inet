//
// Copyright (C) 2015 Andras Varga
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
// Author: Andras Varga
//

#ifndef __INET_ICONTENTIONTX_H
#define __INET_ICONTENTIONTX_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Frame;
class ITxCallback;

/**
 * Abstract interface for processes that implement contention-based channel
 * access. For each frame, it listens on the channel for a DIFS (AIFS) period
 * then for a random backoff period before transitting the frame, and defers when
 * busy channel is sensed. After receiving a corrupted frame, EIFS is used instead
 * of the original DIFS (AIFS).
 *
 * Note that waiting for an ACK (or CTS) and initiating the retransmission if
 * it does not arrive is not handled by this process. Instead, that is typically
 * performed by a frame exchange class inside UpperMac (see IFrameExchange, IUpperMac).
 */
class IContentionTx
{
    public:
        virtual ~IContentionTx() {}
        virtual void transmitContentionFrame(Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount, ITxCallback *completionCallback) = 0;

        // notifications
        virtual void mediumStateChanged(bool mediumFree) = 0;
        virtual void radioTransmissionFinished() = 0;
        virtual void lowerFrameReceived(bool isFcsOk) = 0;
};

void collectContentionTxModules(cModule *firstContentionTxModule, IContentionTx **& contentionTx);

} // namespace ieee80211
} // namespace inet

#endif

