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
#include "inet/common/ShortBitVector.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaver.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Scrambler.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaver.h"
#include "inet/physicallayer/common/ConvolutionalCoder.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMDefs.h"

namespace inet {
namespace physicallayer {

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

APSKEncoder::APSKEncoder(const APSKCode *code) :
        code(code)
{
    if (code->getScrambling())
        scrambler = new Ieee80211Scrambler(code->getScrambling());
    if (code->getInterleaving())
        interleaver = new Ieee80211Interleaver(code->getInterleaving());
    if (code->getConvCode())
        fecEncoder = new ConvolutionalCoder(code->getConvCode());
}

APSKEncoder::APSKEncoder(const IFECCoder* fecEncoder, const IInterleaver* interleaver, const IScrambler* scrambler) :
        fecEncoder(fecEncoder),
        interleaver(interleaver),
        scrambler(scrambler)
{
    const ConvolutionalCode *fec = NULL;
    if (fecEncoder)
        fec = dynamic_cast<const ConvolutionalCode *>(fecEncoder->getForwardErrorCorrection());
    const Ieee80211Interleaving *interleaving = NULL;
    if (interleaver)
        interleaving = dynamic_cast<const Ieee80211Interleaving *>(interleaver->getInterleaving());
    const Ieee80211Scrambling *scrambling = NULL;
    if (scrambler)
        scrambling = dynamic_cast<const Ieee80211Scrambling *>(scrambler->getScrambling());
    code = new APSKCode(fec, interleaving, scrambling);
}

APSKEncoder::APSKEncoder() :
        fecEncoder(NULL),
        interleaver(NULL),
        scrambler(NULL),
        code(NULL)
{
}

APSKEncoder::~APSKEncoder()
{
    delete fecEncoder;
    delete interleaver;
    delete scrambler;
    delete code;
}

} /* namespace physicallayer */
} /* namespace inet */

