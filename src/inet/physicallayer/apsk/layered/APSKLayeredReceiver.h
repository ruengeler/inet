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

#ifndef __INET_APSKLAYEREDRECEIVER_H
#define __INET_APSKLAYEREDRECEIVER_H

#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/contract/IRadioMedium.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/base/SNIRReceiverBase.h"
#include "inet/physicallayer/common/ConvolutionalCode.h"
#include "inet/physicallayer/contract/layered/IDecoder.h"
#include "inet/physicallayer/contract/layered/IDemodulator.h"
#include "inet/physicallayer/contract/layered/IPulseFilter.h"
#include "inet/physicallayer/contract/layered/IAnalogDigitalConverter.h"
#include "inet/physicallayer/contract/IErrorModel.h"
#include "inet/physicallayer/contract/layered/ILayeredErrorModel.h"
#include "inet/physicallayer/apsk/layered/APSKRadioFrame_m.h"

namespace inet {

namespace physicallayer {

class INET_API APSKLayeredReceiver : public SNIRReceiverBase
{
    public:
        enum LevelOfDetail
        {
            BIT_DOMAIN,
            SYMBOL_DOMAIN,
            SAMPLE_DOMAIN,
        };

    protected:
        LevelOfDetail levelOfDetail;
        const ILayeredErrorModel *errorModel;
        const IDecoder *decoder;
        const IDemodulator *demodulator;
        const IPulseFilter *pulseFilter;
        const IAnalogDigitalConverter *analogDigitalConverter;

        W energyDetection;
        W sensitivity;
        Hz carrierFrequency;
        Hz bandwidth;
        double snirThreshold;

    protected:
        virtual void initialize(int stage);
        virtual APSKRadioFrame *deserialize(const BitVector *bits) const;

    public:
        APSKLayeredReceiver();

        virtual void printToStream(std::ostream& stream) const { stream << "APSKLayeredReceiver"; }
        virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception) const;
        virtual const IListening* createListening(const IRadio* radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const;
        virtual const IListeningDecision* computeListeningDecision(const IListening* listening, const IInterference* interference) const;
        virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_APSKLAYEREDRECEIVER_H

