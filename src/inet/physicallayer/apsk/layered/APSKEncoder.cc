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

#include "inet/physicallayer/apsk/layered/APSKEncoder.h"

namespace inet {

namespace physicallayer {

Define_Module (APSKEncoder);

APSKEncoder::APSKEncoder() :
        code(NULL), serializer(NULL), scrambler(NULL), fecEncoder(NULL), interleaver(NULL), headerBitrate(bps(NaN))
{
}

void APSKEncoder::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        serializer = dynamic_cast<ISerializer *>(getSubmodule("serializer")); // FIXME
        scrambler = dynamic_cast<IScrambler *>(getSubmodule("scrambler"));
        fecEncoder = dynamic_cast<IFECCoder *>(getSubmodule("fecEncoder"));
        interleaver = dynamic_cast<IInterleaver *>(getSubmodule("interleaver"));
    }
}

const ITransmissionBitModel* APSKEncoder::encode(const ITransmissionPacketModel* packetModel) const
{
    const BitVector *serializedPacket = packetModel->getSerializedPacket();
    BitVector *encodedBits = new BitVector(*serializedPacket);
    const IScrambling *scrambling = NULL;
    if (scrambler)
    {
        *encodedBits = scrambler->scramble(*encodedBits);
        scrambling = scrambler->getScrambling();
        EV_DEBUG << "Scrambled bits are: " << *encodedBits << endl;
    }
    const IForwardErrorCorrection *forwardErrorCorrection = NULL;
    if (fecEncoder)
    {
        *encodedBits = fecEncoder->encode(*encodedBits);
        forwardErrorCorrection = fecEncoder->getForwardErrorCorrection();
        EV_DEBUG << "FEC encoded bits are: " << *encodedBits << endl;
    }
    const IInterleaving *interleaving = NULL;
    if (interleaver)
    {
        *encodedBits = interleaver->interleave(*encodedBits);
        interleaving = interleaver->getInterleaving();
        EV_DEBUG << "Interleaved bits are: " << *encodedBits << endl;
    }
    return new TransmissionBitModel(encodedBits, forwardErrorCorrection, scrambling, interleaving);
}

} /* namespace physicallayer */

} /* namespace inet */

