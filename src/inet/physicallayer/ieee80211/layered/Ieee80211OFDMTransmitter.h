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

#ifndef __INET_IEEE80211LAYEREDTRANSMITTER_H
#define __INET_IEEE80211LAYEREDTRANSMITTER_H

#include "inet/physicallayer/contract/IEncoder.h"
#include "inet/physicallayer/contract/IModulator.h"
#include "inet/physicallayer/contract/IPulseShaper.h"
#include "inet/physicallayer/contract/IDigitalAnalogConverter.h"
#include "inet/physicallayer/contract/ITransmitter.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OFDMTransmitter : public ITransmitter, public cSimpleModule
{
    // TODO: copy
    public:
        enum LevelOfDetail
        {
            BIT_DOMAIN,
            SYMBOL_DOMAIN,
            SAMPLE_DOMAIN,
        };

    protected:
        LevelOfDetail levelOfDetail;
        const IEncoder *signalEncoder;
        const IEncoder *encoder;
        const IModulator *signalModulator;
        const IModulator *modulator;
        const IPulseShaper *pulseShaper;
        const IDigitalAnalogConverter *digitalAnalogConverter;

        bps bitrate;
        Hz bandwidth;
        Hz carrierFrequency;
        Hz channelSpacing;
        W power;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }
        virtual const ITransmissionPacketModel *createPacketModel(const cPacket *macFrame) const;
        const ITransmissionAnalogModel* createAnalogModel(int headerBitLength, double headerBitRate, int payloadBitLength, double payloadBitRate) const;
        BitVector *serialize(const cPacket* packet) const; // FIXME: kludge
        uint8_t getRate(const BitVector* serializedPacket) const; // TODO: copy
        const ITransmissionPacketModel *createSignalFieldPacketModel(const ITransmissionPacketModel *completePacketModel) const;
        const ITransmissionPacketModel *createDataFieldPacketModel(const ITransmissionPacketModel *completePacketModel) const;
        void encodeAndModulate(const ITransmissionPacketModel *fieldPacketModel, const ITransmissionBitModel *&fieldBitModel, const ITransmissionSymbolModel *&fieldSymbolModel, const IEncoder *encoder, const IModulator *modulator, uint8_t rate, bool isSignalField) const;
        const ITransmissionSymbolModel *createSymbolModel(const ITransmissionSymbolModel *signalFieldSymbolModel, const ITransmissionSymbolModel *dataFieldSymbolModel) const;
        const ITransmissionBitModel *createBitModel(const ITransmissionBitModel *signalFieldBitModel, const ITransmissionBitModel *dataFieldBitModel, uint8_t rate) const;
        void padding(BitVector *serializedPacket, unsigned int dataBitsLength, uint8_t rate) const;

    public:
        virtual const ITransmission *createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const;
        virtual const IEncoder *getEncoder() const { return encoder; }
        virtual const IModulator *getModulator() const { return modulator; }
        virtual const IPulseShaper *getPulseShaper() const{ return pulseShaper; }
        virtual const IDigitalAnalogConverter *getDigitalAnalogConverter() const { return digitalAnalogConverter; }
        virtual W getMaxPower() const { return power; }
        const Hz getBandwidth() const { return bandwidth; }
        const Hz getCarrierFrequency() const { return carrierFrequency; }
        virtual void printToStream(std::ostream& stream) const { stream << "Ieee80211OFDMTransmitter"; }
};

} // namespace physicallayer
} // namespace inet

#endif /* __INET_IEEE80211LAYEREDTRANSMITTER_H */
