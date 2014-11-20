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

#include "inet/physicallayer/layered/LayeredErrorModel.h"
#include "inet/physicallayer/contract/IAPSKModulation.h"
#include "inet/physicallayer/layered/LayeredScalarTransmission.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/layered/SignalBitModel.h"
#include "inet/physicallayer/layered/SignalSampleModel.h"
#include "inet/physicallayer/layered/SignalSymbolModel.h"

namespace inet {
namespace physicallayer {

void LayeredErrorModel::corruptBits(BitVector *bits, double ber, int begin, int end) const
{
    for (int i = begin; i != end; i++)
    {
        double p = uniform(0,1);
        if (p <= ber)
            bits->toggleBit(i);
    }
}

const IReceptionBitModel* LayeredErrorModel::computeBitModel(const LayeredScalarTransmission *transmission, const ISNIR* snir) const
{
    const ITransmissionBitModel *transmissionBitModel = transmission->getBitModel();
    int headerBitLength = transmissionBitModel->getHeaderBitLength();
    double headerBitRate = transmissionBitModel->getHeaderBitRate();
    int payloadBitLength = transmissionBitModel->getPayloadBitLength();
    double payloadBitRate = transmissionBitModel->getPayloadBitRate();
    ASSERT(transmission->getSymbolModel() != NULL);
    const TransmissionSymbolModel *symbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
    const IModulation *modulation = symbolModel->getModulation();
    const BitVector *bits = transmissionBitModel->getBits();
    BitVector *corruptedBits = new BitVector(*bits); // FIXME: memory leak
    if (dynamic_cast<const IAPSKModulation *>(modulation))
    {
        const IAPSKModulation *apskModulation = (const IAPSKModulation *) modulation;
        double ber = apskModulation->calculateBER(snir->getMin(), transmission->getBandwidth().get(), payloadBitRate);
        corruptBits(corruptedBits, ber, 0, corruptedBits->getSize()); // TODO: separate signal and header?
    }
    else
        throw cRuntimeError("Unknown modulation");
    return new const ReceptionBitModel(headerBitLength, headerBitRate, payloadBitLength, payloadBitRate, corruptedBits, modulation);
}

const IReceptionSymbolModel* LayeredErrorModel::computeSymbolModel(const LayeredScalarTransmission *transmission, const ISNIR* snir) const
{
    const ITransmissionSymbolModel *transmissionSymbolModel = transmission->getSymbolModel();
    // TODO: SER?
    // TODO: implement symbol error model
    return new ReceptionSymbolModel(transmissionSymbolModel->getSymbolLength(), transmissionSymbolModel->getSymbolRate(), transmissionSymbolModel->getSymbols());
}

void LayeredErrorModel::initialize(int stage)
{
    throw cRuntimeError("Unimplemented");
}

const IReceptionSampleModel* LayeredErrorModel::computeSampleModel(const LayeredScalarTransmission *transmission, const ISNIR* snir) const
{
    const ITransmissionSampleModel *transmissionSampleModel = transmission->getSampleModel();
    int sampleLength = transmissionSampleModel->getSampleLength();
    double sampleRate = transmissionSampleModel->getSampleRate();
    const std::vector<W> *samples = transmissionSampleModel->getSamples();
    W rssi = W(0); // TODO: error model
    return new const ReceptionSampleModel(sampleLength, sampleRate, samples, rssi);
}

const IReceptionPacketModel* LayeredErrorModel::computePacketModel(const LayeredScalarTransmission *transmission, const ISNIR* snir) const
{
    const ITransmissionPacketModel *transmissionPacketModel = transmission->getPacketModel();
    const cPacket *packet = transmissionPacketModel->getPacket();
    double per = -1;
    bool packetErrorless = true;
    if (per != 0)
        packetErrorless = false;
    return new const ReceptionPacketModel(packet, NULL, NULL, NULL, NULL, per, packetErrorless);
}

} /* namespace physicallayer */
} /* namespace inet */

