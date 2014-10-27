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
#include "inet/physicallayer/layered/LayeredTransmission.h"

namespace inet {
namespace physicallayer {

const IReceptionBitModel* LayeredErrorModel::computeBitModel(const ITransmissionBitModel* transmissionBitModel, const ISNIR* snir) const
{
    int bitLength = transmissionBitModel->getBitLength();
    double bitrate = transmissionBitModel->getBitRate();
    const BitVector *bits = transmissionBitModel->getBits();
    const IModulation *modulation = transmissionBitModel->getModulation();
    double ber;
    int bitErrorCount;
    const LayeredTransmission *layeredTransmission = check_and_cast<const LayeredTransmission* >(snir->getReception()->getTransmission());
    if (dynamic_cast<const IAPSKModulation *>(modulation))
    {
        const IAPSKModulation *apskModulation = (const IAPSKModulation *) modulation;
        // TODO: separate header and payload
        ber = apskModulation->calculateBER(snir->getMin(), layeredTransmission->getBandwidth().get(), bitrate);
        bitErrorCount = ber * bitLength;
    }
    else
        throw cRuntimeError("Unknown modulation");
    return new const ReceptionBitModel(bitLength, bitrate, bits, modulation, ber, bitErrorCount);
}

const IReceptionSymbolModel* LayeredErrorModel::computeSymbolModel(const ITransmissionSymbolModel* transmissionSymbolModel, const ISNIR* snir) const
{
    const double ser = -1; // TODO: error model
    const double symbolErrorCount = -1; // TODO: error model
    return new ReceptionSymbolModel(transmissionSymbolModel->getSymbolLength(), transmissionSymbolModel->getSymbolRate(), transmissionSymbolModel->getSymbols(), ser, symbolErrorCount);

}

const IReceptionSampleModel* LayeredErrorModel::computeSampleModel(const ITransmissionSampleModel* tranmssionSampleModel, const ISNIR* snir) const
{
    int sampleLength = tranmssionSampleModel->getSampleLength();
    double sampleRate = tranmssionSampleModel->getSampleRate();
    const std::vector<W> *samples = tranmssionSampleModel->getSamples();
    W rssi = W(0); // TODO: error model
    return new const ReceptionSampleModel(sampleLength, sampleRate, samples, rssi);
}

const IReceptionPacketModel* LayeredErrorModel::computePacketModel(const ITransmissionPacketModel* transmissionPacketModel, const ISNIR* snir) const
{
    const cPacket *packet = transmissionPacketModel->getPacket();
    double per = -1;
    bool packetErrorless = true;
    if (per != 0)
        packetErrorless = false;
// TODO:   forwardErrorCorrection, scrambling, interleaving
    return new const ReceptionPacketModel(packet, NULL, NULL, NULL, per, packetErrorless);
}


} /* namespace physicallayer */
} /* namespace inet */

