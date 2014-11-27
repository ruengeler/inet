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

#include "Ieee80211LayeredDecoderModule.h"
#include "inet/physicallayer/common/DummySerializer.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"

namespace inet {
namespace physicallayer {

#define OFDM_SYMBOL_SIZE 48
Define_Module(Ieee80211LayeredDecoderModule);

void Ieee80211LayeredDecoderModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        descrambler = dynamic_cast<Ieee80211Scrambler *>(getSubmodule("descrambler"));
        fecDecoder = dynamic_cast<ConvolutionalCoder *>(getSubmodule("fecDecoder"));
        deinterleaver = dynamic_cast<Ieee80211Interleaver *>(getSubmodule("fecDecoder"));
        channelSpacing = Hz(par("channelSpacing"));
        layeredDecoder = new Ieee80211LayeredDecoder(descrambler , fecDecoder, deinterleaver, channelSpacing);
    }
}

const IReceptionPacketModel* Ieee80211LayeredDecoderModule::decode(const IReceptionBitModel* bitModel) const
{
    return layeredDecoder->decode(bitModel);
}

Ieee80211LayeredDecoderModule::~Ieee80211LayeredDecoderModule()
{
    delete layeredDecoder;
}

} /* namespace physicallayer */
} /* namespace inet */
