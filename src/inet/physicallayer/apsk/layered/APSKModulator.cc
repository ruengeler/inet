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
    const BitVector *bits = bitModel->getBits();
    unsigned int codeWordLength = modulation->getCodeWordLength();
    int symbolLength = (bitModel->getHeaderBitLength() + bitModel->getPayloadBitLength() + codeWordLength - 1) / codeWordLength;
//    const double symbolRate = bitModel->getBitRate() / nBPSC;
    ShortBitVector symbolBits;
    std::vector<const ISymbol*> *symbols = new std::vector<const ISymbol*>(); // FIXME: Sample model should delete it
    for (unsigned int i = 0; i < bits->getSize(); i++) {
        symbolBits.setBit(i % codeWordLength, bits->getBit(i));
        if (i % codeWordLength == codeWordLength - 1)
            symbols->push_back(modulation->mapToConstellationDiagram(symbolBits));
    }
    return new TransmissionSymbolModel(symbolLength, 0, symbols, modulation); // FIXME: symbol length, symbol rate
}

} // namespace physicallayer

} // namespace inet

