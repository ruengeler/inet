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

#include "Ieee80211OFDMDemodulatorModule.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211OFDMDemodulatorModule);

void Ieee80211OFDMDemodulatorModule::initialize(int stage)
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
        ofdmDemodulator = new Ieee80211OFDMDemodulator(demodulationScheme);
    }
}

const IReceptionBitModel *Ieee80211OFDMDemodulatorModule::demodulate(const IReceptionSymbolModel *symbolModel) const
{
    return ofdmDemodulator->demodulate(symbolModel);
}

Ieee80211OFDMDemodulatorModule::~Ieee80211OFDMDemodulatorModule()
{
    delete ofdmDemodulator;
}

} /* namespace physicallayer */
} /* namespace inet */
