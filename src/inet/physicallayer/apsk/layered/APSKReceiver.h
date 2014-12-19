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

#ifndef __INET_APSKRECEIVER_H
#define __INET_APSKRECEIVER_H

#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/contract/IRadioMedium.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/base/SNIRReceiverBase.h"
#include "inet/physicallayer/contract/layered/IDecoder.h"
#include "inet/physicallayer/contract/layered/IDemodulator.h"
#include "inet/physicallayer/contract/layered/IPulseFilter.h"
#include "inet/physicallayer/contract/layered/IAnalogDigitalConverter.h"
#include "inet/physicallayer/contract/IErrorModel.h"
#include "inet/physicallayer/contract/layered/ILayeredErrorModel.h"

namespace inet {
namespace physicallayer {

class INET_API APSKReceiver : public SNIRReceiverBase
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
        const IDecoder *headerDecoder;
        const IDemodulator *demodulator;
        const IDemodulator *headerDemodulator;
        const IPulseFilter *pulseFilter;
        const IAnalogDigitalConverter *analogDigitalConverter;

        W energyDetection;
        W sensitivity;
        Hz carrierFrequency;
        Hz bandwidth;
        double snirThreshold;

    protected:
        virtual void initialize(int stage);
        unsigned int getSignalFieldLength(const BitVector *signalField) const;
        unsigned int calculatePadding(unsigned int dataFieldLengthInBits, const APSKModulationBase *modulationScheme, const ConvolutionalCode *fec) const;

        const IReceptionPacketModel *createCompleteReceptionPacketModel(const IReceptionPacketModel *signalFieldReceptionPacketModel, const IReceptionPacketModel *dataFieldReceptionPacketModel) const;
        const IReceptionSymbolModel *createSignalFieldReceptionSymbolModel(const IReceptionSymbolModel *receptionSymbolModel) const;
        const IReceptionSymbolModel *createDataFieldReceptionSymbolModel(const IReceptionSymbolModel *receptionSymbolModel) const;
        const IReceptionBitModel *createDataFieldReceptionBitModel(const APSKModulationBase *demodulationScheme, const ConvolutionalCode *convCode, const IReceptionBitModel *receptionBitModel, const IReceptionPacketModel *signalFieldReceptionPacketModel) const;
        const IReceptionBitModel *createSignalFieldReceptionBitModel(const IReceptionBitModel *receptionBitModel) const;
        const IReceptionPacketModel *demodulateAndDecodeSignalField(const IRadioMedium *medium, const IRadio *receiver, const LayeredTransmission *transmission, const IReceptionSymbolModel *&receptionSymbolModel,  const IReceptionBitModel *&receptionBitModel) const;
        const IReceptionPacketModel *demodulateAndDecodeDataField(const IReceptionSymbolModel* receptionSymbolModel, const IReceptionBitModel* receptionBitModel, const IReceptionPacketModel *signalFieldReceptionPacketModel) const;

    public:
        bool computeIsReceptionPossible(const IListening *listening, const IReception *reception) const;
        const IListeningDecision* computeListeningDecision(const IListening* listening, const IInterference* interference) const;
        const IListening* createListening(const IRadio* radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const;
        virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const;
        APSKReceiver();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_APSKRECEIVER_H */
