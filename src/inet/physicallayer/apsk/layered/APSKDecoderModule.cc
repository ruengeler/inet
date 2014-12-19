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

#include "APSKDecoderModule.h"
#include "inet/physicallayer/common/DummySerializer.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"

namespace inet {
namespace physicallayer {

Define_Module(APSKDecoderModule);

void APSKDecoderModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        descrambler = dynamic_cast<const IScrambler *>(getSubmodule("descrambler"));
        fecDecoder = dynamic_cast<const IFECCoder *>(getSubmodule("fecDecoder"));
        deinterleaver = dynamic_cast<const IInterleaver *>(getSubmodule("deinterleaver"));
        channelSpacing = Hz(par("channelSpacing"));
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER)
    {
        layeredDecoder = new APSKDecoder(descrambler , fecDecoder, deinterleaver, channelSpacing);
    }
}

const IReceptionPacketModel* APSKDecoderModule::decode(const IReceptionBitModel* bitModel) const
{
    return layeredDecoder->decode(bitModel);
}

APSKDecoderModule::~APSKDecoderModule()
{
    delete layeredDecoder;
}

} /* namespace physicallayer */
} /* namespace inet */
