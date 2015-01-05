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

#include "inet/physicallayer/apsk/layered/APSKDemodulator.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/apsk/layered/APSKSymbol.h"
#include "inet/physicallayer/layered/SignalBitModel.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKDemodulator);

APSKDemodulator::APSKDemodulator() :
    modulation(nullptr)
{
}

void APSKDemodulator::initialize(int stage)
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

BitVector APSKDemodulator::demodulateSymbol(const APSKSymbol *signalSymbol) const
{
    std::vector<const APSKSymbol*> apskSymbols; // TODO: signalSymbol->getSubCarrierSymbols();
    BitVector field;
    for (unsigned int i = 0; i < apskSymbols.size(); i++)
    {
        if (!isPilotOrDcSubcarrier(i))
        {
            const APSKSymbol *apskSymbol = apskSymbols.at(i);
            ShortBitVector bits = modulation->demapToBitRepresentation(apskSymbol);
            for (unsigned int j = 0; j < bits.getSize(); j++)
                field.appendBit(bits.getBit(j));
        }
    }
    EV_DEBUG << "The field symbols has been demodulated into the following bit stream: " << field << endl;
    return field;
}

bool APSKDemodulator::isPilotOrDcSubcarrier(int i) const
{
   return i == 5 || i == 19 || i == 33 || i == 47 || i == 26; // pilots are: 5,19,33,47, 26 (0+26) is a dc subcarrier
}


const IReceptionBitModel* APSKDemodulator::demodulate(const IReceptionSymbolModel* symbolModel) const
{
    const std::vector<const ISymbol*> *symbols = symbolModel->getSymbols();
    BitVector *bitRepresentation = new BitVector();
    for (unsigned int i = 0; i < symbols->size(); i++)
    {
        const APSKSymbol *symbol = dynamic_cast<const APSKSymbol *>(symbols->at(i));
        BitVector bits = demodulateSymbol(symbol);
        for (unsigned int j = 0; j < bits.getSize(); j++)
            bitRepresentation->appendBit(bits.getBit(j));
    }
    return new ReceptionBitModel(0, 0, 0, 0, bitRepresentation, modulation);
}

} // namespace physicallayer

} // namespace inet

