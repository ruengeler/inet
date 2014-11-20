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

#include "inet/physicallayer/ieee80211/layered/Ieee80211LayeredEncoder.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211PHYFrame_m.h"
#include "inet/common/ShortBitVector.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaver.h"

namespace inet {
namespace physicallayer {

#define PPDU_SERVICE_FIELD_BITS_LENGTH 16
#define PPDU_TAIL_BITS_LENGTH 6
#define OFDM_SYMBOL_SIZE 48

Define_Module(Ieee80211LayeredEncoder);

void Ieee80211LayeredEncoder::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        serializer = dynamic_cast<ISerializer *>(getSubmodule("serializer")); // FIXME
        scrambler = dynamic_cast<IScrambler *>(getSubmodule("scrambler"));
        dataFECEncoder = dynamic_cast<IFECCoder *>(getSubmodule("fecEncoder"));
        signalFECEncoder = dynamic_cast<IFECCoder *>(getSubmodule("signalFECEncoder"));
        interleaver = dynamic_cast<IInterleaver *>(getSubmodule("interleaver")); // FIXME this should not be a module
        signalInterleaver = dynamic_cast<IInterleaver *>(getSubmodule("signalInterleaver"));
        channelSpacing = Hz(par("channelSpacing"));
        headerBitrate = computeHeaderBitRate();
    }
}

BitVector Ieee80211LayeredEncoder::signalFieldEncode(const BitVector& signalField) const
{
    // NOTE: The contents of the SIGNAL field are not scrambled.
    ShortBitVector signalFieldRate = calculateRateField(channelSpacing, headerBitrate);
    BitVector fecEncodedBits;
    if (signalFECEncoder) // non-compliant mode
        fecEncodedBits = signalFECEncoder->encode(signalField);
    else
    {
        const Ieee80211ConvolutionalCode *fec = getFecFromSignalFieldRate(signalFieldRate);
        ConvolutionalCoder fecCoder(fec);
        fecEncodedBits = fecCoder.encode(signalField);
    }
    EV_DEBUG << "FEC encoded bits of the SIGNAL field are: " << fecEncodedBits << endl;
    BitVector interleavedBits;
    if (signalInterleaver) // non-compliant mode
        interleavedBits = signalInterleaver->interleave(fecEncodedBits);
    else
    {
        const IModulation *modulationScheme = getModulationFromSignalFieldRate(signalFieldRate);
        const Ieee80211Interleaving *interleaving = getInterleavingFromModulation(modulationScheme);
        Ieee80211Interleaver interleaver(interleaving);
        interleavedBits = interleaver.interleave(fecEncodedBits);
    }
    EV_DEBUG << "Interleaved bits of the SIGNAL field are: " << interleavedBits << endl;
    return interleavedBits;
}

BitVector Ieee80211LayeredEncoder::dataFieldEncode(const BitVector& dataField, const ShortBitVector& signalFieldRate) const
{
   BitVector scrambledBits = scrambler->scramble(dataField);
   EV_DEBUG << "Scrambled bits of the DATA field are: " << scrambledBits << endl;
   BitVector fecEncodedBits;
   if (dataFECEncoder) // non-compliant mode
       fecEncodedBits = dataFECEncoder->encode(scrambledBits);
   else
   {
       const Ieee80211ConvolutionalCode *fec = getFecFromSignalFieldRate(signalFieldRate);
       ConvolutionalCoder fecCoder(fec);
       fecEncodedBits = fecCoder.encode(scrambledBits);
   }
   EV_DEBUG << "FEC encoded bits of the DATA field are: " << fecEncodedBits << endl;
   BitVector interleavedBits;
   if (interleaver) // non-compliant mode
       interleavedBits = interleaver->interleave(fecEncodedBits);
   else
   {
       const IModulation *modulationScheme = getModulationFromSignalFieldRate(signalFieldRate);
       const Ieee80211Interleaving *interleaving = getInterleavingFromModulation(modulationScheme);
       Ieee80211Interleaver interleaver(interleaving);
       interleavedBits = interleaver.interleave(fecEncodedBits);
   }
   EV_DEBUG << "Interleaved bits of the DATA field are: " << interleavedBits << endl;
   return interleavedBits;
}

// TODO: copy from Ieee80211LayereDecoder, factor this code out
const APSKModulationBase* Ieee80211LayeredEncoder::getModulationFromSignalFieldRate(const ShortBitVector& rate) const
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

// TODO: copy from Ieee80211LayereDecoder
const Ieee80211ConvolutionalCode* Ieee80211LayeredEncoder::getFecFromSignalFieldRate(const ShortBitVector& rate) const
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

// FIXME: HACK
BitVector Ieee80211LayeredEncoder::serialize(const cPacket* packet) const
{
    // HACK: Here we just compute the bit-correct PLCP header
    // and then we fill the remaining with random bits
    const Ieee80211PHYFrame *phyFrame = check_and_cast<const Ieee80211PHYFrame*>(packet);
    BitVector serializedPacket;
    // RATE, 4 bits
    ShortBitVector rate(phyFrame->getRate(), 4);
    for (unsigned int i = 0; i < rate.getSize(); i++)
        serializedPacket.appendBit(rate.getBit(i));
    // Reserved, 1 bit
    serializedPacket.appendBit(0); // Bit 4 is reserved. It shall be set to 0 on transmit and ignored on receive.
    // Length, 12 bits
    ShortBitVector byteLength(phyFrame->getLength(), 12); // == macFrame->getByteLength()
    for (unsigned int i = 0; i < byteLength.getSize(); i++)
        serializedPacket.appendBit(byteLength.getBit(i));
    // Parity, 1 bit
    serializedPacket.appendBit(0); // whatever (at least for now)
    // Tail, 6 bit
    serializedPacket.appendBit(0, 6); // The bits 18–23 constitute the SIGNAL TAIL field, and all 6 bits shall be set to 0
    // Service, 16 bit
    // The bits from 0–6 of the SERVICE field, which are transmitted first, are set to 0s
    // and are used to synchronize the descrambler in the receiver. The remaining 9 bits
    // (7–15) of the SERVICE field shall be reserved for future use. All reserved bits shall
    // be set to 0.
    serializedPacket.appendBit(0, 16);
    ASSERT(serializedPacket.getSize() == 40);
    for (unsigned int i = 0; i < byteLength.toDecimal() * 8; i++)
        serializedPacket.appendBit(rand() % 2);
    serializedPacket.appendBit(0, 6); // tail bits
    // FIXME: HACK, append bits
    int dataBitsLength = PPDU_TAIL_BITS_LENGTH + PPDU_SERVICE_FIELD_BITS_LENGTH + byteLength.toDecimal() * 8;
    const APSKModulationBase *modulationScheme = getModulationFromSignalFieldRate(rate);
    unsigned int codedBitsPerOFDMSymbol = modulationScheme->getCodeWordLength() * OFDM_SYMBOL_SIZE;
    const Ieee80211ConvolutionalCode *fec = getFecFromSignalFieldRate(rate);
    int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol * fec->getCodeRatePuncturingK() / fec->getCodeRatePuncturingN();
    int appendedBitsLength = dataBitsPerOFDMSymbol - dataBitsLength % dataBitsPerOFDMSymbol;
    serializedPacket.appendBit(0, appendedBitsLength);
    return serializedPacket;
}
// FIXME: HACK

// TODO: Factor this code out
bps Ieee80211LayeredEncoder::computeDataBitRate(const BitVector& serializedPacket) const
{
    ShortBitVector rate = getRate(serializedPacket);
    if (channelSpacing == MHz(20))
    {
        if (rate == ShortBitVector("1101"))
            return bps(6000000);
        else if (rate == ShortBitVector("1111"))
            return bps(9000000);
        else if (rate == ShortBitVector("0101"))
            return bps(12000000);
        else if (rate == ShortBitVector("0111"))
            return bps(18000000);
        else if (rate == ShortBitVector("1001"))
            return bps(24000000);
        else if (rate == ShortBitVector("1011"))
            return bps(36000000);
        else if (rate == ShortBitVector("0001"))
            return bps(48000000);
        else if (rate == ShortBitVector("0011"))
            return bps(54000000);
        else
            throw cRuntimeError("Invalid rate field: %d", rate.toDecimal());
    }
    else if (channelSpacing == MHz(10))
    {
        if (rate == ShortBitVector("1101"))
            return bps(3000000);
        else if (rate == ShortBitVector("1111"))
            return bps(4500000);
        else if (rate == ShortBitVector("0101"))
            return bps(6000000);
        else if (rate == ShortBitVector("0111"))
            return bps(9000000);
        else if (rate == ShortBitVector("1001"))
            return bps(12000000);
        else if (rate == ShortBitVector("1011"))
            return bps(18000000);
        else if (rate == ShortBitVector("0001"))
            return bps(24000000);
        else if (rate == ShortBitVector("0011"))
            return bps(27000000);
        else
            throw cRuntimeError("Invalid rate field: %d", rate.toDecimal());
    }
    else if (channelSpacing == MHz(5))
    {
       if (rate == ShortBitVector("1101"))
           return bps(1500000);
       else if (rate == ShortBitVector("1111"))
           return bps(2250000);
       else if (rate == ShortBitVector("0101"))
           return bps(3000000);
       else if (rate == ShortBitVector("0111"))
           return bps(4500000);
       else if (rate == ShortBitVector("1001"))
           return bps(6000000);
       else if (rate == ShortBitVector("1011"))
           return bps(9000000);
       else if (rate == ShortBitVector("0001"))
           return bps(12000000);
       else if (rate == ShortBitVector("0011"))
           return bps(13500000);
       else
           throw cRuntimeError("Invalid rate field: %d", rate.toDecimal());
    }
    else
        throw cRuntimeError("Unknown channel spacing = %lf", channelSpacing);
    return bps(0);
}

// FIXME: copy
// move this code to the Ieee80211Modulation class
ShortBitVector Ieee80211LayeredEncoder::calculateRateField(Hz channelSpacing, bps bitrate) const
{
    if (channelSpacing == MHz(20))
    {
        if (bitrate == bps(6000000))
            return ShortBitVector("1101");
        else if (bitrate == bps(9000000))
            return ShortBitVector("1111");
        else if (bitrate == bps(12000000))
            return ShortBitVector("0101");
        else if (bitrate == bps(18000000))
            return ShortBitVector("0111");
        else if (bitrate == bps(24000000))
            return ShortBitVector("1001");
        else if (bitrate == bps(36000000))
            return ShortBitVector("1011");
        else if (bitrate == bps(48000000))
            return ShortBitVector("0001");
        else if (bitrate == bps(54000000))
            return ShortBitVector("0011");
        else
            throw cRuntimeError("%lf Hz channel spacing does not support %lf bps bitrate", channelSpacing.get(), bitrate.get());
    }
    else if (channelSpacing == MHz(10))
    {
        if (bitrate == bps(3000000))
            return ShortBitVector("1101");
        else if (bitrate == bps(4500000))
            return ShortBitVector("1111");
        else if (bitrate == bps(6000000))
            return ShortBitVector("0101");
        else if (bitrate == bps(9000000))
            return ShortBitVector("0111");
        else if (bitrate == bps(12000000))
            return ShortBitVector("1001");
        else if (bitrate == bps(18000000))
            return ShortBitVector("1011");
        else if (bitrate == bps(24000000))
            return ShortBitVector("0001");
        else if (bitrate == bps(27000000))
            return ShortBitVector("0011");
        else
            throw cRuntimeError("%lf Hz channel spacing does not support %lf bps bitrate", channelSpacing.get(), bitrate.get());
    }
    else if (channelSpacing == MHz(5))
    {
       if (bitrate == bps(1500000))
           return ShortBitVector("1101");
       else if (bitrate == bps(2250000))
           return ShortBitVector("1111");
       else if (bitrate == bps(3000000))
           return ShortBitVector("0101");
       else if (bitrate == bps(4500000))
           return ShortBitVector("0111");
       else if (bitrate == bps(6000000))
           return ShortBitVector("1001");
       else if (bitrate == bps(9000000))
           return ShortBitVector("1011");
       else if (bitrate == bps(12000000))
           return ShortBitVector("0001");
       else if (bitrate == bps(13500000))
           return ShortBitVector("0011");
       else
           throw cRuntimeError("%lf Hz channel spacing does not support %lf bps bitrate", channelSpacing.get(), bitrate.get());
    }
    else
        throw cRuntimeError("Unknown channel spacing = %lf", channelSpacing);
    return ShortBitVector("0000");
}

// TODO: copy
const Ieee80211Interleaving* Ieee80211LayeredEncoder::getInterleavingFromModulation(const IModulation *modulationScheme) const
{
    const IAPSKModulation *dataModulationScheme = dynamic_cast<const IAPSKModulation*>(modulationScheme);
    ASSERT(dataModulationScheme != NULL);
    return new Ieee80211Interleaving(dataModulationScheme->getCodeWordLength() * OFDM_SYMBOL_SIZE, dataModulationScheme->getCodeWordLength()); // FIXME: memory leak
}

// TODO: move this code to the Ieee80211Modulation class
bps Ieee80211LayeredEncoder::computeHeaderBitRate() const
{
    // TODO: Revise, these are the minimum bitrates for each channel spacing.
    if (channelSpacing == MHz(20))
        return bps(6000000);
    else if (channelSpacing == MHz(10))
        return bps(3000000);
    else if (channelSpacing == MHz(5))
        return bps(1500000);
    else
        throw cRuntimeError("Invalid channel spacing %lf", channelSpacing.get());
}

ShortBitVector Ieee80211LayeredEncoder::getRate(const BitVector& serializedPacket) const
{
    ShortBitVector rate;
    for (unsigned int i = 0; i < 4; i++)
        rate.appendBit(serializedPacket.getBit(i));
    return rate;
}

const ITransmissionBitModel* Ieee80211LayeredEncoder::encode(const ITransmissionPacketModel* packetModel) const
{
    const cPacket *packet = packetModel->getPacket();
//    BitVector serializedPacket = serializer->serialize(packet);
    BitVector serializedPacket = serialize(packet); // FIXME: HACK, this packet needs a ieee80211 serializer
    // The SIGNAL field is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1), Tail (6),
    // fields, so the SIGNAL field is 24 bits long.
    BitVector signalField;
    for (int i = 0; i < 24; i++)
        signalField.appendBit(serializedPacket.getBit(i));
    BitVector dataField;
    // NOTE: the SERVICE field, which is part of the PLCP header goes to the DATA field.
    // Then we apply different FEC encodings (and later modulations) - not for the whole PLCP header
    // but for the SIGNAL field - then for the DATA field.
    for (unsigned int i = 24; i < serializedPacket.getSize(); i++)
        dataField.appendBit(serializedPacket.getBit(i));
    ShortBitVector signalFieldRate = getRate(serializedPacket);
    BitVector encodedSignalField = signalFieldEncode(signalField);
    BitVector encodedDataField = dataFieldEncode(dataField, signalFieldRate);
    BitVector *encodedBits = new BitVector();
    for (unsigned int i = 0; i < encodedSignalField.getSize(); i++)
        encodedBits->appendBit(encodedSignalField.getBit(i));
    for (unsigned int i = 0; i < encodedDataField.getSize(); i++)
        encodedBits->appendBit(encodedDataField.getBit(i));
    double dataBitRate = computeDataBitRate(serializedPacket).get();
    return new TransmissionBitModel(encodedSignalField.getSize(), encodedDataField.getSize(), headerBitrate.get(), dataBitRate, encodedBits, dataFECEncoder->getForwardErrorCorrection(), scrambler->getScrambling(), interleaver->getInterleaving());
}

} /* namespace physicallayer */
} /* namespace inet */
