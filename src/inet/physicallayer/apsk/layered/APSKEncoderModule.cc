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

#include "APSKEncoderModule.h"

namespace inet {
namespace physicallayer {

Define_Module(APSKEncoderModule);

void APSKEncoderModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        serializer = dynamic_cast<ISerializer *>(getSubmodule("serializer")); // FIXME
        scrambler = dynamic_cast<IScrambler *>(getSubmodule("scrambler"));
        fecEncoder = dynamic_cast<IFECCoder *>(getSubmodule("fecEncoder"));
        interleaver = dynamic_cast<IInterleaver *>(getSubmodule("interleaver"));
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER)
    {
        encoder = new APSKEncoder(fecEncoder, interleaver, scrambler);
    }
}

const ITransmissionBitModel* APSKEncoderModule::encode(const ITransmissionPacketModel* packetModel) const
{
    return encoder->encode(packetModel);
}

APSKEncoderModule::~APSKEncoderModule()
{
    delete encoder;
}

} /* namespace physicallayer */
} /* namespace inet */
