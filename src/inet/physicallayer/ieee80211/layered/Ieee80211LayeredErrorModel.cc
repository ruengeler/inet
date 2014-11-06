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

#include "inet/physicallayer/ieee80211/layered/Ieee80211LayeredErrorModel.h"
#include "inet/physicallayer/contract/IAPSKModulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/layered/LayeredTransmission.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/layered/SignalBitModel.h"
#include "inet/physicallayer/layered/SignalSampleModel.h"
#include "inet/physicallayer/layered/SignalSymbolModel.h"
#include "inet/physicallayer/modulation/OFDMSymbol.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211LayeredErrorModel);

const IReceptionBitModel* Ieee80211LayeredErrorModel::computeBitModel(const LayeredTransmission *transmission, const ISNIR* snir) const
{
    const ITransmissionBitModel *transmissionBitModel = transmission->getBitModel();
    int signalBitLength = transmissionBitModel->getHeaderBitLength();
    double signalBitRate = transmissionBitModel->getHeaderBitRate();
    int dataBitLength = transmissionBitModel->getPayloadBitLength();
    double dataBitRate = transmissionBitModel->getPayloadBitRate();
    ASSERT(transmission->getSymbolModel() != NULL);
    const TransmissionSymbolModel *symbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
    const IModulation *modulation = symbolModel->getModulation();
    const BitVector *bits = transmissionBitModel->getBits();
    BitVector *corruptedBits = new BitVector(*bits); // FIXME: memory leak
    if (dynamic_cast<const IAPSKModulation *>(modulation))
    {
        const IAPSKModulation *apskModulation = (const IAPSKModulation *) modulation;
        double dataFieldBer = apskModulation->calculateBER(snir->getMin(), transmission->getBandwidth().get(), dataBitRate);
        corruptBits(corruptedBits, dataFieldBer, signalBitLength, corruptedBits->getSize());
        double signalFieldBer = BPSKModulation::singleton.calculateBER(snir->getMin(), transmission->getBandwidth().get(), signalBitRate);
        corruptBits(corruptedBits, signalFieldBer, 0, signalBitLength);
    }
    else
        throw cRuntimeError("Unknown modulation");
    return new const ReceptionBitModel(signalBitLength, dataBitLength, signalBitRate, dataBitRate, corruptedBits, modulation);
}

const IReceptionSymbolModel* Ieee80211LayeredErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISNIR* snir) const
{
    // TODO: implement error model
    const TransmissionSymbolModel *transmissionSymbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
    const IModulation *modulation = transmissionSymbolModel->getModulation();
    const APSKModulationBase *apskModulation = check_and_cast<const APSKModulationBase *>(modulation);
//    http://www.dsplog.com/2012/01/01/symbol-error-rate-16qam-64qam-256qam/
    double ser = 0.001; // FIXME: modulation->calculateSER();
    const std::vector<const ISymbol*> *symbols = transmissionSymbolModel->getSymbols();
    unsigned int constellationSize = apskModulation->getConstellationSize();
    std::vector<const ISymbol*> *corruptedSymbols = new std::vector<const ISymbol *>(); // FIXME: memory leak
    const APSKSymbol *constellationDiagram = apskModulation->getEncodingTable();
    for (unsigned int i = 0; i < symbols->size(); i++)
    {
        double p = uniform(0,1);
        if (p <= ser)
        {
            int corruptedSymbolIndex = intuniform(0, constellationSize - 1); // TODO: it can be equal to the current symbol
            const ISymbol *corruptedSymbol = &constellationDiagram[corruptedSymbolIndex];
            corruptedSymbols->push_back(corruptedSymbol);
        }
        else
            corruptedSymbols->push_back(symbols->at(i));
    }

    return new ReceptionSymbolModel(transmissionSymbolModel->getSymbolLength(), transmissionSymbolModel->getSymbolRate(), corruptedSymbols);
}

void Ieee80211LayeredErrorModel::initialize(int stage)
{

}

const IReceptionSampleModel* Ieee80211LayeredErrorModel::computeSampleModel(const LayeredTransmission *transmission, const ISNIR* snir) const
{
    const ITransmissionSampleModel *transmissionSampleModel = transmission->getSampleModel();
    int sampleLength = transmissionSampleModel->getSampleLength();
    double sampleRate = transmissionSampleModel->getSampleRate();
    const std::vector<W> *samples = transmissionSampleModel->getSamples();
    W rssi = W(0); // TODO: error model
    return new const ReceptionSampleModel(sampleLength, sampleRate, samples, rssi);
}

const IReceptionPacketModel* Ieee80211LayeredErrorModel::computePacketModel(const LayeredTransmission *transmission, const ISNIR* snir) const
{
    // TODO: implement error model
    const ITransmissionPacketModel *transmissionPacketModel = transmission->getPacketModel();
    const cPacket *packet = transmissionPacketModel->getPacket();
    double per = -1;
    bool packetErrorless = true;
    if (per != 0)
        packetErrorless = false;
    return new const ReceptionPacketModel(packet, NULL, NULL, NULL, per, packetErrorless);
}

} /* namespace physicallayer */
} /* namespace inet */

