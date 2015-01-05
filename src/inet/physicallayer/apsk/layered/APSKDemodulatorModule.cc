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

#include "APSKDemodulatorModule.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/apsk/layered/APSKSymbol.h"
#include "inet/physicallayer/layered/SignalBitModel.h"

namespace inet {
namespace physicallayer {

Define_Module(APSKDemodulatorModule);

void APSKDemodulatorModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        const APSKModulationBase *demodulationScheme = NULL;
        const char *modulationSchemeStr = par("demodulationScheme");
        if (!strcmp("QAM-16", modulationSchemeStr))
            demodulationScheme = &QAM16Modulation::singleton;
        else if (!strcmp("QAM-64", modulationSchemeStr))
            demodulationScheme = &QAM64Modulation::singleton;
        else if (!strcmp("QPSK", modulationSchemeStr))
            demodulationScheme = &QPSKModulation::singleton;
        else if (!strcmp("BPSK", modulationSchemeStr))
            demodulationScheme = &BPSKModulation::singleton;
        else
            throw cRuntimeError("Unknown modulation scheme = %s", modulationSchemeStr);
    }
}

BitVector APSKDemodulatorModule::demodulateSymbol(const APSKSymbol *signalSymbol) const
{
    std::vector<const APSKSymbol*> apskSymbols; // TODO: signalSymbol->getSubCarrierSymbols();
    BitVector field;
    for (unsigned int i = 0; i < apskSymbols.size(); i++)
    {
        if (!isPilotOrDcSubcarrier(i))
        {
            const APSKSymbol *apskSymbol = apskSymbols.at(i);
            ShortBitVector bits = demodulationScheme->demapToBitRepresentation(apskSymbol);
            for (unsigned int j = 0; j < bits.getSize(); j++)
                field.appendBit(bits.getBit(j));
        }
    }
    EV_DEBUG << "The field symbols has been demodulated into the following bit stream: " << field << endl;
    return field;
}

const IReceptionBitModel* APSKDemodulatorModule::createBitModel(const BitVector *bitRepresentation, int signalFieldLength, double signalFieldBitRate, int dataFieldLength, double dataFieldBitRate) const
{
    return new ReceptionBitModel(signalFieldLength, signalFieldBitRate, dataFieldLength, dataFieldBitRate, bitRepresentation, demodulationScheme);
}

bool APSKDemodulatorModule::isPilotOrDcSubcarrier(int i) const
{
   return i == 5 || i == 19 || i == 33 || i == 47 || i == 26; // pilots are: 5,19,33,47, 26 (0+26) is a dc subcarrier
}


const IReceptionBitModel* APSKDemodulatorModule::demodulate(const IReceptionSymbolModel* symbolModel) const
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
    double dataFieldBitRate = 0;
    double signalFieldBitRate = 0;
    return createBitModel(bitRepresentation, 0, 0, 0, 0); // TODO:
}

} // namespace physicallayer

} // namespace inet
