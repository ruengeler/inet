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

#include "inet/physicallayer/ieee80211/errormodel/layered/Ieee80211OFDMErrorModel.h"
#include "inet/physicallayer/contract/IAPSKModulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/layered/SignalBitModel.h"
#include "inet/physicallayer/layered/SignalSampleModel.h"
#include "inet/physicallayer/layered/SignalSymbolModel.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211OFDMErrorModel);

void Ieee80211OFDMErrorModel::initialize(int stage)
{

}

const IReceptionBitModel* Ieee80211OFDMErrorModel::computeBitModel(const LayeredTransmission *transmission, const ISNIR* snir) const
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
        const ScalarTransmissionSignalAnalogModel *analogModel = dynamic_cast<const ScalarTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
        double dataFieldBer = apskModulation->calculateBER(snir->getMin(), analogModel->getBandwidth().get(), dataBitRate);
        corruptBits(corruptedBits, dataFieldBer, signalBitLength, corruptedBits->getSize());
        double signalFieldBer = BPSKModulation::singleton.calculateBER(snir->getMin(), analogModel->getBandwidth().get(), signalBitRate);
        corruptBits(corruptedBits, signalFieldBer, 0, signalBitLength);
    }
    else
        throw cRuntimeError("Unknown modulation");
    return new const ReceptionBitModel(signalBitLength, dataBitLength, signalBitRate, dataBitRate, corruptedBits);
}

const IReceptionSymbolModel* Ieee80211OFDMErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISNIR* snir) const
{
    const TransmissionSymbolModel *transmissionSymbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
    const IModulation *modulation = transmissionSymbolModel->getModulation();
    const APSKModulationBase *dataModulation = check_and_cast<const APSKModulationBase *>(modulation);
    unsigned int dataFieldConstellationSize = dataModulation->getConstellationSize();
    unsigned int signalFieldConstellationSize = BPSKModulation::singleton.getConstellationSize();
    double headerSER = BPSKModulation::singleton.calculateSER(snir->getMin());
    double dataSER = dataModulation->calculateSER(snir->getMin());
    const APSKSymbol *constellationDiagramForDataField = dataModulation->getEncodingTable();
    const APSKSymbol *constellationDiagramForSignalField = BPSKModulation::singleton.getEncodingTable();
    const std::vector<const ISymbol*> *symbols = transmissionSymbolModel->getSymbols();
    std::vector<const ISymbol*> *corruptedSymbols = new std::vector<const ISymbol *>(); // FIXME: memory leak
    // Only the first symbol is signal field symbol
    corruptedSymbols->push_back(corruptOFDMSymbol(check_and_cast<const Ieee80211OFDMSymbol *>(symbols->at(0)), headerSER, signalFieldConstellationSize, constellationDiagramForSignalField));
    // The remaining are all data field symbols
    for (unsigned int i = 1; i < symbols->size(); i++)
    {
        Ieee80211OFDMSymbol *corruptedOFDMSymbol = corruptOFDMSymbol(check_and_cast<const Ieee80211OFDMSymbol *>(symbols->at(i)), dataSER,
                                                            dataFieldConstellationSize, constellationDiagramForDataField);
        corruptedSymbols->push_back(corruptedOFDMSymbol);
    }
    return new ReceptionSymbolModel(transmissionSymbolModel->getSymbolLength(), transmissionSymbolModel->getSymbolRate(), corruptedSymbols);
}

void Ieee80211OFDMErrorModel::corruptBits(BitVector *bits, double ber, int begin, int end) const
{
    for (int i = begin; i != end; i++)
    {
        double p = uniform(0,1);
        if (p <= ber)
            bits->toggleBit(i);
    }
}

Ieee80211OFDMSymbol *Ieee80211OFDMErrorModel::corruptOFDMSymbol(const Ieee80211OFDMSymbol *symbol, double ser, int constellationSize, const APSKSymbol *constellationDiagram) const
{
    std::vector<const APSKSymbol *> subcarrierSymbols = symbol->getSubCarrierSymbols();
    for (int j = 0; j < symbol->symbolSize(); j++)
    {
        double p = uniform(0,1);
        if (p <= ser)
        {
            int corruptedSubcarrierSymbolIndex = intuniform(0, constellationSize - 1); // TODO: it can be equal to the current symbol
            const APSKSymbol *corruptedSubcarrierSymbol = &constellationDiagram[corruptedSubcarrierSymbolIndex];
            subcarrierSymbols[j] = corruptedSubcarrierSymbol;
        }
    }
    return new Ieee80211OFDMSymbol(subcarrierSymbols);
}

const IReceptionSampleModel* Ieee80211OFDMErrorModel::computeSampleModel(const LayeredTransmission *transmission, const ISNIR* snir) const
{
    throw cRuntimeError("Unimplemented!");
    // TODO: implement sample error model
    const ITransmissionSampleModel *transmissionSampleModel = transmission->getSampleModel();
    int sampleLength = transmissionSampleModel->getSampleLength();
    double sampleRate = transmissionSampleModel->getSampleRate();
    const std::vector<W> *samples = transmissionSampleModel->getSamples();
    W rssi = W(0); // TODO: error model
    return new const ReceptionSampleModel(sampleLength, sampleRate, samples, rssi);
}

const IReceptionPacketModel* Ieee80211OFDMErrorModel::computePacketModel(const LayeredTransmission *transmission, const ISNIR* snir) const
{
    throw cRuntimeError("Unimplemented!");
    // TODO: implement error model
    const ITransmissionPacketModel *transmissionPacketModel = transmission->getPacketModel();
    const cPacket *packet = transmissionPacketModel->getPacket();
    double per = 0.0;
    bool packetErrorless = per == 0.0;
    return new const ReceptionPacketModel(packet, NULL, NULL, NULL, NULL, per, packetErrorless);
}

} /* namespace physicallayer */
} /* namespace inet */

