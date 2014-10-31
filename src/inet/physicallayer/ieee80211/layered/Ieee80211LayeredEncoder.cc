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

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211LayeredEncoder);

void Ieee80211LayeredEncoder::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        serializer = check_and_cast<ISerializer *>(getSubmodule("serializer"));
        scrambler = check_and_cast<IScrambler *>(getSubmodule("scrambler"));
        dataFECEncoder = check_and_cast<IFECCoder *>(getSubmodule("fecEncoder"));
        signalFECEncoder = check_and_cast<IFECCoder *>(getSubmodule("signalFECEncoder"));
        interleaver = check_and_cast<IInterleaver *>(getSubmodule("interleaver"));
        signalInterleaver = check_and_cast<IInterleaver *>(getSubmodule("signalInterleaver"));
    }
}

BitVector Ieee80211LayeredEncoder::signalFieldEncode(const BitVector& signalField) const
{
    // NOTE: The contents of the SIGNAL field are not scrambled.
    BitVector fecEncodedBits = signalFECEncoder->encode(signalField);
    EV_DEBUG << "FEC encoded bits of the SIGNAL field are: " << fecEncodedBits << endl;
    BitVector interleavedBits = signalInterleaver->interleave(fecEncodedBits);
    EV_DEBUG << "Interleaved bits of the SIGNAL field are: " << interleavedBits << endl;
    return interleavedBits;
}

BitVector Ieee80211LayeredEncoder::dataFieldEncode(const BitVector& dataField) const
{
   BitVector scrambledBits = scrambler->scramble(dataField);
   EV_DEBUG << "Scrambled bits of the DATA field are: " << scrambledBits << endl;
   BitVector fecEncodedBits = dataFECEncoder->encode(scrambledBits);
   EV_DEBUG << "FEC encoded bits of the DATA field are: " << fecEncodedBits << endl;
   BitVector interleavedBits = interleaver->interleave(fecEncodedBits);
   EV_DEBUG << "Interleaved bits of the DATA field are: " << interleavedBits << endl;
   return interleavedBits;
}

// FIXME: HACK
BitVector Ieee80211LayeredEncoder::serialize(const cPacket* packet) const
{
    // HACK: Here we just compute the bit-correct PLCP header
    // and then we fill the remaining with random bits
    const Ieee80211PHYFrame *phyFrame = check_and_cast<const Ieee80211PHYFrame*>(packet);
    BitVector serializedPacket;
    // RATE, 4 bits
    ShortBitVector rate(phyFrame->getRate());
    for (unsigned int i = 0; i < rate.getSize(); i++)
        serializedPacket.appendBit(rate.getBit(i));
    // Reserved, 1 bit
    serializedPacket.appendBit(0); // Bit 4 is reserved. It shall be set to 0 on transmit and ignored on receive.
    // Length, 12 bits
    ShortBitVector length(phyFrame->getLength(), 12); // == macFrame->getByteLength()
    for (unsigned int i = 0; i < length.getSize(); i++)
        serializedPacket.appendBit(length.getBit(i));
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
    ASSERT(serializedPacket.getSize() != 40);
    for (unsigned int i = 0; i < length.toDecimal() * 8; i++)
        serializedPacket.appendBit(rand() % 2);
    return serializedPacket;
}
// FIXME: HACK

double Ieee80211LayeredEncoder::getBitRate(const BitVector& packet) const
{
    throw cRuntimeError("Unimplemented");
    return -1;
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
    BitVector encodedSignalField = signalFieldEncode(signalField);
    BitVector encodedDataField = dataFieldEncode(dataField);
    BitVector *encodedBits = new BitVector();
    for (unsigned int i = 0; i < encodedSignalField.getSize(); i++)
        encodedBits->appendBit(encodedSignalField.getBit(i));
    for (unsigned int i = 0; i < encodedDataField.getSize(); i++)
        encodedBits->appendBit(encodedDataField.getBit(i));
    double dataFieldBitRate = 36000000; // FIXME: this is not a constant, we set it to 36Mb/s for testing purposes
    double signalFieldBitRate = 1000000; // The PLCP header and preamble are always transmitted at 1 Mb/s
    return new TransmissionBitModel(encodedDataField.getSize(), dataFieldBitRate, encodedSignalField.getSize(), signalFieldBitRate, encodedBits, dataFECEncoder->getForwardErrorCorrection(), scrambler->getScrambling(), interleaver->getInterleaving());
}

} /* namespace physicallayer */
} /* namespace inet */
