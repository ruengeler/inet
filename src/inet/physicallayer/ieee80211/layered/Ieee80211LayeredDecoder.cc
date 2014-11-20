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


Ieee80211LayeredDecoder::Ieee80211LayeredDecoder(const Ieee80211Scrambler *descrambler, const ConvolutionalCoder *fecDecoder, const Ieee80211Interleaver *deinterleaver) :
        descrambler(descrambler),
        fecDecoder(fecDecoder),
        deinterleaver(deinterleaver)
{

}

const Ieee80211ConvolutionalCode* Ieee80211LayeredDecoder::getFecFromSignalFieldRate(const ShortBitVector& rate) const
{
    // Table 18-6—Contents of the SIGNAL field
    // Table 18-4—Modulation-dependent parameters
    // FIXME: memory leaks
    if (rate == ShortBitVector("1101") || rate == ShortBitVector("0101") || rate == ShortBitVector("1001"))
        return new Ieee80211ConvolutionalCode(1, 2);
    else if (rate == ShortBitVector("1111") || rate == ShortBitVector("0111") || rate == ShortBitVector("1011") ||
             rate == ShortBitVector("0111"))
        return new Ieee80211ConvolutionalCode(3, 4);
    else if (rate == ShortBitVector("0001"))
        return new Ieee80211ConvolutionalCode(2, 3);
    else
        throw cRuntimeError("Unknown rate field  = %s", rate.toString().c_str());
}

const APSKModulationBase* Ieee80211LayeredDecoder::getModulationFromSignalFieldRate(const ShortBitVector& rate) const
{
    // Table 18-6—Contents of the SIGNAL field
    // Table 18-4—Modulation-dependent parameters
    if (rate == ShortBitVector("1101") || rate == ShortBitVector("1111"))
        return &BPSKModulation::singleton;
    else if (rate == ShortBitVector("0101") || rate == ShortBitVector("0111"))
        return &QPSKModulation::singleton;
    else if (rate == ShortBitVector("1001") || rate == ShortBitVector("1011"))
        return &QAM16Modulation::singleton;
    else if(rate == ShortBitVector("0001") || rate == ShortBitVector("0011"))
        return &QAM64Modulation::singleton;
    else
        throw cRuntimeError("Unknown rate field = %s", rate.toString().c_str());
}

//const IReceptionPacketModel* Ieee80211LayeredDecoder::decode(const IReceptionBitModel* bitModel) const
//{
//    const BitVector *bits = bitModel->getBits();
//    BitVector signalField;
//    for (unsigned int i = 0; i < ENCODED_SIGNAL_FIELD_LENGTH; i++)
//        signalField.appendBit(bits->getBit(i));
//    BitVector decodedSignalField = decodeSignalField(signalField);
//    ShortBitVector signalFieldRate = getSignalFieldRate(decodedSignalField);
//    const Ieee80211ConvolutionalCode *fec = getFecFromSignalFieldRate(signalFieldRate);
//    const IModulation *modulationScheme = bitModel->getModulation();
//    ASSERT(modulationScheme != NULL);
//    const Ieee80211Interleaving *deinterleaving = getInterleavingFromModulation(modulationScheme);
//    Ieee80211Interleaver deinterleaver(deinterleaving);
//    ConvolutionalCoder fecDecoder(fec);
//    unsigned int psduLengthInBits = getSignalFieldLength(decodedSignalField) * 8;
//    unsigned int dataFieldLengthInBits = psduLengthInBits + PPDU_SERVICE_FIELD_BITS_LENGTH + PPDU_TAIL_BITS_LENGTH;
//    dataFieldLengthInBits += calculatePadding(dataFieldLengthInBits, modulationScheme, fec);
//    ASSERT(dataFieldLengthInBits % fec->getCodeRatePuncturingK() == 0);
//    unsigned int encodedDataFieldLengthInBits = dataFieldLengthInBits * fec->getCodeRatePuncturingN() / fec->getCodeRatePuncturingK();
//    if (dataFieldLengthInBits + ENCODED_SIGNAL_FIELD_LENGTH > bits->getSize())
//        throw cRuntimeError("The calculated data field length = %d is greater then the actual bitvector length = %d", dataFieldLengthInBits, bits->getSize());
//    BitVector dataField;
//    for (unsigned int i = 0; i < encodedDataFieldLengthInBits; i++)
//        dataField.appendBit(bits->getBit(ENCODED_SIGNAL_FIELD_LENGTH+i));
//    BitVector decodedDataField = decodeDataField(dataField, fecDecoder, deinterleaver);
//    BitVector decodedBits;
//    for (unsigned int i = 0; i < decodedSignalField.getSize(); i++)
//        decodedBits.appendBit(decodedSignalField.getBit(i));
//    for (unsigned int i = 0; i < decodedDataField.getSize(); i++)
//        decodedBits.appendBit(decodedDataField.getBit(i));
//    return createPacketModel(decodedBits, descrambler->getScrambling(), fec, deinterleaving);
//}

const IReceptionPacketModel* Ieee80211LayeredDecoder::decode(const IReceptionBitModel* bitModel) const
{
    const BitVector *fieldBits = bitModel->getBits();
    BitVector deinterleavedDataField = deinterleaver->deinterleave(*fieldBits);
    std::pair<BitVector, bool> fecDecodedDataField = fecDecoder->decode(deinterleavedDataField);
    bool isDecodedSuccessfully = fecDecodedDataField.second;
    if (!isDecodedSuccessfully)
        throw cRuntimeError("FEC error"); // TODO: implement correct error handling
    if (descrambler)
        return createPacketModel(descrambler->descramble(fecDecodedDataField.first), descrambler->getScrambling(), fecDecoder->getForwardErrorCorrection(), deinterleaver->getInterleaving());
    return createPacketModel(fecDecodedDataField.first, descrambler->getScrambling(), fecDecoder->getForwardErrorCorrection(), deinterleaver->getInterleaving());
}

const Ieee80211Interleaving* Ieee80211LayeredDecoder::getInterleavingFromModulation(const IModulation *modulationScheme) const
{
    const IAPSKModulation *dataModulationScheme = dynamic_cast<const IAPSKModulation*>(modulationScheme);
    ASSERT(dataModulationScheme != NULL);
    return new Ieee80211Interleaving(dataModulationScheme->getCodeWordLength() * OFDM_SYMBOL_SIZE, dataModulationScheme->getCodeWordLength()); // FIXME: memory leak
}

// TODO: remove decodedBits
const IReceptionPacketModel* Ieee80211LayeredDecoder::createPacketModel(const BitVector& decodedBits, const Ieee80211Scrambling *scrambling, const ConvolutionalCode *fec, const Ieee80211Interleaving *interleaving) const
{
    double per = -1;
    bool packetErrorless = true; // TODO: compute packet error rate, packetErrorLess
    return new ReceptionPacketModel(NULL, new BitVector(decodedBits), fec, scrambling, interleaving, per, packetErrorless); // FIXME: memory leak
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
    delete deinterleaver;
    delete descrambler;
    delete fecDecoder;
}

} /* namespace physicallayer */
} /* namespace inet */
