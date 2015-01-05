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

#include "inet/physicallayer/apsk/layered/APSKDecoder.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKDecoder);

APSKDecoder::APSKDecoder() :
    code(nullptr),
    descrambler(nullptr),
    fecDecoder(nullptr),
    deinterleaver(nullptr)
{
}

void APSKDecoder::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        descrambler = dynamic_cast<const IScrambler *>(getSubmodule("descrambler"));
        fecDecoder = dynamic_cast<const IFECCoder *>(getSubmodule("fecDecoder"));
        deinterleaver = dynamic_cast<const IInterleaver *>(getSubmodule("deinterleaver"));
    }
}

const IReceptionPacketModel* APSKDecoder::decode(const IReceptionBitModel* bitModel) const
{
    BitVector *decodedBits = new BitVector(*bitModel->getBits());
    const IInterleaving *interleaving = nullptr;
    if (deinterleaver)
    {
        *decodedBits = deinterleaver->deinterleave(*decodedBits);
        interleaving = deinterleaver->getInterleaving();
    }
    const IForwardErrorCorrection *forwardErrorCorrection = nullptr;
    if (fecDecoder)
    {
        std::pair<BitVector, bool> fecDecodedDataField = fecDecoder->decode(*decodedBits);
        bool isDecodedSuccessfully = fecDecodedDataField.second;
        if (!isDecodedSuccessfully)
            throw cRuntimeError("FEC error"); // TODO: implement correct error handling
        *decodedBits = fecDecodedDataField.first;
        forwardErrorCorrection = fecDecoder->getForwardErrorCorrection();
    }
    const IScrambling *scrambling = nullptr;
    if (descrambler)
    {
        scrambling = descrambler->getScrambling();
        *decodedBits = descrambler->descramble(*decodedBits);
    }
    double per = -1;
    bool packetErrorless = true; // TODO: compute packet error rate, packetErrorLess
    return new ReceptionPacketModel(nullptr, decodedBits, forwardErrorCorrection, scrambling, interleaving, per, packetErrorless); // FIXME: memory leak
}

} // namespace physicallayer

} // namespace inet

