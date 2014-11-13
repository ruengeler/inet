//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_LAYEREDRECEIVER_H
#define __INET_LAYEREDRECEIVER_H

#include "inet/physicallayer/contract/IDecoder.h"
#include "inet/physicallayer/contract/IDemodulator.h"
#include "inet/physicallayer/contract/IPulseFilter.h"
#include "inet/physicallayer/contract/IAnalogDigitalConverter.h"
#include "inet/physicallayer/contract/IErrorModel.h"
#include "inet/physicallayer/base/SNIRReceiverBase.h"

namespace inet {

namespace physicallayer {

class INET_API LayeredReceiver: public SNIRReceiverBase
{
  protected:
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

  public:
    LayeredReceiver();

    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const;
    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const;
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const;
    bool computeIsReceptionPossible(const IListening *listening, const IReception *reception) const;

    virtual const IDecoder *getDecoder() const { return decoder; }
    virtual const IDemodulator *getDemodulator() const { return demodulator; }
    virtual const IPulseFilter *getPulseFilter() const { return pulseFilter; }
    virtual const IAnalogDigitalConverter *getAnalogDigitalConverter() const { return analogDigitalConverter; }

    virtual W getMinReceptionPower() const { return sensitivity; }
    virtual Hz getCarrierFrequency() const { return carrierFrequency; }
    virtual void setCarrierFrequency(Hz carrierFrequency) { this->carrierFrequency = carrierFrequency; }
    virtual void setBandwidth(Hz bandwidth) { this->bandwidth = bandwidth; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_LAYEREDRECEIVER_H
