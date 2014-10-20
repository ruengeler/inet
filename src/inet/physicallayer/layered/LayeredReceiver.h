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
#include "inet/physicallayer/scalar/ScalarReceiver.h"

namespace inet {

namespace physicallayer {

class INET_API LayeredReceiver : public ScalarReceiver
{
  protected:
    const ILayeredErrorModel *errorModel;
    const IDecoder *decoder;
    const IDemodulator *demodulator;
    const IPulseFilter *pulseFilter;
    const IAnalogDigitalConverter *analogDigitalConverter;

  protected:
    virtual void initialize(int stage);

    /*
     * These functions do the "transmission to reception" conversations.
     * If the actual radio architecture does not implement some domains you should
     * implement to corresponding function that can bridge these missing domains.
     */
    virtual const IReceptionPacketModel *createReceptionPacketModel(const ITransmissionPacketModel *packetModel) const;
    virtual const IReceptionBitModel *createReceptionBitModel(const ITransmissionBitModel *bitModel) const;
    virtual const IReceptionSampleModel *createReceptionSampleModel(const ITransmissionSampleModel *sampleModel) const;
    virtual const IReceptionSymbolModel *createReceptionSymbolModel(const ITransmissionSymbolModel *symbolModel) const;
    virtual const IReceptionAnalogModel *createReceptionAnalogModel(const ITransmissionAnalogModel *analogModel, const IReception *reception, const IListening *listening, const IInterference *interference) const;

    virtual const ISNIR *computeSNIR(const IReception *reception, const IListening *listening, const IInterference *interference) const;
    const INoise *computeNoise(const IListening *listening, const IInterference *interference) const;

  public:
    LayeredReceiver();

    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const;

    virtual const IDecoder *getDecoder() const { return decoder; }
    virtual const IDemodulator *getDemodulator() const { return demodulator; }
    virtual const IPulseFilter *getPulseFilter() const { return pulseFilter; }
    virtual const IAnalogDigitalConverter *getAnalogDigitalConverter() const { return analogDigitalConverter; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_LAYEREDRECEIVER_H
