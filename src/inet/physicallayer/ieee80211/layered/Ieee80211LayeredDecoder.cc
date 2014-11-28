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

#include "inet/physicallayer/ieee80211/layered/Ieee80211LayeredDecoder.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/contract/IAPSKModulation.h"
#include "inet/physicallayer/common/DummySerializer.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"

namespace inet {
namespace physicallayer {

#define OFDM_SYMBOL_SIZE 48
#define ENCODED_SIGNAL_FIELD_LENGTH 48
// Table L-7—Bit assignment for SIGNAL field
#define SIGNAL_RATE_FIELD_START 0
#define SIGNAL_RATE_FIELD_END 3
#define SIGNAL_LENGTH_FIELD_START 5
#define SIGNAL_LENGTH_FIELD_END 16
#define SIGNAL_PARITY_FIELD 17
#define PPDU_SERVICE_FIELD_BITS_LENGTH 16
#define PPDU_TAIL_BITS_LENGTH 6

Ieee80211LayeredDecoder::Ieee80211LayeredDecoder(const Ieee80211OFDMCode *code) :
        descrambler(NULL),
        fecDecoder(NULL),
        deinterleaver(NULL)
{
    this->code = code;
    if (code->getScrambling())
        descrambler = new Ieee80211Scrambler(code->getScrambling());
    if (code->getConvCode())
        fecDecoder = new ConvolutionalCoder(code->getConvCode());
    if (code->getInterleaving())
        deinterleaver = new Ieee80211Interleaver(code->getInterleaving());
}

Ieee80211LayeredDecoder::Ieee80211LayeredDecoder(const IScrambler *descrambler, const IFECCoder *fecDecoder, const IInterleaver *deinterleaver, Hz channelSpacing) :
        descrambler(descrambler),
        fecDecoder(fecDecoder),
        deinterleaver(deinterleaver),
        channelSpacing(channelSpacing)
{
    const Ieee80211ConvolutionalCode *fec = NULL;
    if (fecDecoder)
        fec = dynamic_cast<const Ieee80211ConvolutionalCode *>(fecDecoder->getForwardErrorCorrection());
    const Ieee80211Interleaving *interleaving = NULL;
    if (deinterleaver)
        interleaving = dynamic_cast<const Ieee80211Interleaving *>(deinterleaver->getInterleaving());
    const Ieee80211Scrambling *scrambling = NULL;
    if (descrambler)
        scrambling = dynamic_cast<const Ieee80211Scrambling *>(descrambler->getScrambling());
    code = new Ieee80211OFDMCode(fec, interleaving, scrambling, channelSpacing);
}

const IReceptionPacketModel* Ieee80211LayeredDecoder::decode(const IReceptionBitModel* bitModel) const
{
    BitVector *decodedBits = new BitVector(*bitModel->getBits());
    const IInterleaving *interleaving = NULL;
    if (deinterleaver)
    {
        *decodedBits = deinterleaver->deinterleave(*decodedBits);
        interleaving = deinterleaver->getInterleaving();
    }
    const IForwardErrorCorrection *forwardErrorCorrection = NULL;
    if (fecDecoder)
    {
        std::pair<BitVector, bool> fecDecodedDataField = fecDecoder->decode(*decodedBits);
        bool isDecodedSuccessfully = fecDecodedDataField.second;
        if (!isDecodedSuccessfully)
            throw cRuntimeError("FEC error"); // TODO: implement correct error handling
        *decodedBits = fecDecodedDataField.first;
        forwardErrorCorrection = fecDecoder->getForwardErrorCorrection();
    }
    const IScrambling *scrambling = NULL;
    if (descrambler)
    {
        scrambling = descrambler->getScrambling();
        *decodedBits = descrambler->descramble(*decodedBits);
    }
    return createPacketModel(decodedBits, scrambling, forwardErrorCorrection, interleaving);
}

const IReceptionPacketModel* Ieee80211LayeredDecoder::createPacketModel(const BitVector *decodedBits, const IScrambling *scrambling, const IForwardErrorCorrection *fec, const IInterleaving *interleaving) const
{
    double per = -1;
    bool packetErrorless = true; // TODO: compute packet error rate, packetErrorLess
    return new ReceptionPacketModel(NULL, decodedBits, fec, scrambling, interleaving, per, packetErrorless); // FIXME: memory leak
}

ShortBitVector Ieee80211LayeredDecoder::getSignalFieldRate(const BitVector& signalField) const
{
    ShortBitVector rate;
    for (int i = SIGNAL_RATE_FIELD_START; i <= SIGNAL_RATE_FIELD_END; i++)
        rate.appendBit(signalField.getBit(i));
    return rate;
}

unsigned int Ieee80211LayeredDecoder::getSignalFieldLength(const BitVector& signalField) const
{
    ShortBitVector length;
    for (int i = SIGNAL_LENGTH_FIELD_START; i <= SIGNAL_LENGTH_FIELD_END; i++)
        length.appendBit(signalField.getBit(i));
    return length.toDecimal();
}

unsigned int Ieee80211LayeredDecoder::calculatePadding(unsigned int dataFieldLengthInBits, const IModulation *modulationScheme, const Ieee80211ConvolutionalCode *fec) const
{
    const IAPSKModulation *dataModulationScheme = dynamic_cast<const IAPSKModulation*>(modulationScheme);
    ASSERT(dataModulationScheme != NULL);
    unsigned int codedBitsPerOFDMSymbol = dataModulationScheme->getCodeWordLength() * OFDM_SYMBOL_SIZE;
    unsigned int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol * fec->getCodeRatePuncturingK() / fec->getCodeRatePuncturingN();
    return dataBitsPerOFDMSymbol - dataFieldLengthInBits % dataBitsPerOFDMSymbol;
}

Ieee80211LayeredDecoder::~Ieee80211LayeredDecoder()
{
    delete code;
    delete deinterleaver;
    delete descrambler;
    delete fecDecoder;
}

} /* namespace physicallayer */
} /* namespace inet */
