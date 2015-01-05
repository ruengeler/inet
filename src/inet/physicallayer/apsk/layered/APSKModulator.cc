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

#include "inet/physicallayer/apsk/layered/APSKModulator.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKModulator);

APSKModulator::APSKModulator() :
    modulation(nullptr)
{
}

void APSKModulator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        const char *modulationString = par("modulation");
        if (!strcmp("QAM-16", modulationString))
            modulation = &QAM16Modulation::singleton;
        else if (!strcmp("QAM-64", modulationString))
            modulation = &QAM64Modulation::singleton;
        else if (!strcmp("QPSK", modulationString))
            modulation = &QPSKModulation::singleton;
        else if (!strcmp("BPSK", modulationString))
            modulation = &BPSKModulation::singleton;
        else
            throw cRuntimeError("Unknown modulation = %s", modulationString);
    }
}

const ITransmissionSymbolModel *APSKModulator::modulate(const ITransmissionBitModel *bitModel) const
{
    std::vector<const ISymbol*> *symbols = new std::vector<const ISymbol*>(); // FIXME: Sample model should delete it
    const BitVector *bits = bitModel->getBits();
    // Divide the resulting coded and interleaved data string into groups of N_BPSC bits.
    unsigned int nBPSC = modulation->getCodeWordLength();
// TODO:    const int symbolLength = preambleSymbolLength + (bitModel->getBitLength() + nBPSC - 1) / nBPSC;
//    const double symbolRate = bitModel->getBitRate() / nBPSC;
    ShortBitVector bitGroup;
    std::vector<const APSKSymbol *> apskSymbols;
    for (unsigned int i = 0; i < bits->getSize(); i++)
    {
        // For each of the bit groups, convert the bit group into a complex number according
        // to the modulation encoding tables
        bitGroup.setBit(i % nBPSC, bits->getBit(i));
        if (i % nBPSC == nBPSC - 1)
        {
            const APSKSymbol *apskSymbol = modulation->mapToConstellationDiagram(bitGroup);
            apskSymbols.push_back(apskSymbol);
        }
    }
    // Divide the complex number string into groups of 48 complex numbers.
    // Each such group is associated with one OFDM symbol.
//    APSKSymbol *ofdmSymbol = new APSKSymbol(); // TODO: fix memory leak
//    int symbolID = 1;
//    for (unsigned int i = 0; i < apskSymbols.size(); i++)
//    {
//        int subcarrierIndex = getSubcarrierIndex(i % OFDM_SYMBOL_SIZE);
//// TODO:        ofdmSymbol->pushAPSKSymbol(apskSymbols.at(i), subcarrierIndex);
//        // In each group, the complex numbers are numbered 0 to 47 and mapped hereafter into OFDM
//        // subcarriers numbered -26 to -22, -20 to -8, -6 to -1, 1 to 6, 8 to 20, and 22 to 26.
//        // The 0 subcarrier, associated with center frequency, is omitted and filled with the value 0.
//        if (i % OFDM_SYMBOL_SIZE == OFDM_SYMBOL_SIZE - 1)
//        {
//            insertPilotSubcarriers(ofdmSymbol, symbolID);
//            EV_DEBUG << "Modulated #" << symbolID << " DATA field: " << *ofdmSymbol << endl;
//            symbols->push_back(ofdmSymbol);
//            ofdmSymbol = new APSKSymbol();
//            symbolID++;
//        }
//    }
    return new TransmissionSymbolModel(0, 0, symbols, modulation); // FIXME: symbol length, symbol rate
}

} // namespace physicallayer

} // namespace inet

