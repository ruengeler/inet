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

#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMDemodulator.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/apsk/layered/APSKSymbol.h"
#include "inet/physicallayer/common/layered/SignalBitModel.h"

namespace inet {
namespace physicallayer {

Ieee80211OFDMDemodulator::Ieee80211OFDMDemodulator(const Ieee80211OFDMModulation *ofdmModulation) :
        ofdmModulation(ofdmModulation),
        demodulationScheme(ofdmModulation->getModulationScheme())
{

}

Ieee80211OFDMDemodulator::Ieee80211OFDMDemodulator(const APSKModulationBase* demodulationScheme) :
        ofdmModulation(NULL),
        demodulationScheme(demodulationScheme)
{

}

BitVector Ieee80211OFDMDemodulator::demodulateSymbol(const Ieee80211OFDMSymbol *signalSymbol) const
{
    std::vector<const APSKSymbol*> apskSymbols = signalSymbol->getSubCarrierSymbols();
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

const IReceptionBitModel* Ieee80211OFDMDemodulator::createBitModel(const BitVector *bitRepresentation, int signalFieldLength, double signalFieldBitRate, int dataFieldLength, double dataFieldBitRate) const
{
    return new ReceptionBitModel(signalFieldLength, signalFieldBitRate, dataFieldLength, dataFieldBitRate, bitRepresentation);
}

bool Ieee80211OFDMDemodulator::isPilotOrDcSubcarrier(int i) const
{
   return i == 5 || i == 19 || i == 33 || i == 47 || i == 26; // pilots are: 5,19,33,47, 26 (0+26) is a dc subcarrier
}


const IReceptionBitModel* Ieee80211OFDMDemodulator::demodulate(const IReceptionSymbolModel* symbolModel) const
{
    const std::vector<const ISymbol*> *symbols = symbolModel->getSymbols();
    BitVector *bitRepresentation = new BitVector();
    for (unsigned int i = 0; i < symbols->size(); i++)
    {
        const Ieee80211OFDMSymbol *symbol = dynamic_cast<const Ieee80211OFDMSymbol *>(symbols->at(i));
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
