//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_SIGNALBITMODEL_H
#define __INET_SIGNALBITMODEL_H

#include <vector>
#include "inet/common/BitVector.h"
#include "inet/physicallayer/contract/ISignalBitModel.h"

namespace inet {

namespace physicallayer {

class INET_API SignalBitModel : public virtual ISignalBitModel
{
  protected:
    const BitVector *bits;
    const int headerBitLength;
    const double headerBitRate;
    const int payloadBitLength;
    const double payloadBitRate;

  public:
    SignalBitModel() :
        bits(&BitVector::UNDEF),
        headerBitLength(-1),
        headerBitRate(sNaN),
        payloadBitLength(-1),
        payloadBitRate(sNaN)
    {}

    SignalBitModel(int headerBitLength, int payloadBitLength, double headerBitRate, double payloadBitRate, const BitVector *bits) :
        bits(bits),
        headerBitLength(headerBitLength),
        headerBitRate(headerBitRate),
        payloadBitLength(payloadBitLength),
        payloadBitRate(payloadBitRate)
    {}

    virtual void printToStream(std::ostream &stream) const;
    virtual int getHeaderBitLength() const { return headerBitLength; }
    virtual double getHeaderBitRate() const { return headerBitRate; }
    virtual int getPayloadBitLength() const { return payloadBitLength; }
    virtual double getPayloadBitRate() const { return payloadBitRate; }
    virtual const BitVector* getBits() const { return bits; }
};

class INET_API TransmissionBitModel : public SignalBitModel, public virtual ITransmissionBitModel
{
  protected:
    const IForwardErrorCorrection *forwardErrorCorrection;
    const IScrambling *scrambling;
    const IInterleaving *interleaving;

  public:
    TransmissionBitModel() :
        SignalBitModel()
    {}

    TransmissionBitModel(int headerBitLength, int payloadBitLength, double headerBitRate, double payloadBitRate, const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scramblerInfo, const IInterleaving *interleaverInfo) :
        SignalBitModel(headerBitLength, payloadBitLength, headerBitRate, payloadBitRate, bits),
        forwardErrorCorrection(forwardErrorCorrection),
        scrambling(scramblerInfo),
        interleaving(interleaverInfo)
    {}

    virtual const IForwardErrorCorrection *getForwardErrorCorrection() const { return forwardErrorCorrection; }
    virtual const IScrambling *getScrambling() const { return scrambling; }
    virtual const IInterleaving *getInterleaving() const { return interleaving; }
};

class INET_API ReceptionBitModel : public SignalBitModel, public virtual IReceptionBitModel
{
  protected:
    const IModulation *modulation;

  public:
    ReceptionBitModel() :
        SignalBitModel(),
        modulation(NULL)
    {}

    ReceptionBitModel(int headerBitLength, int payloadBitLength, double headerBitRate, double payloadBitRate, const BitVector *bits, const IModulation *modulation) :
        SignalBitModel(headerBitLength, payloadBitLength, headerBitRate, payloadBitRate, bits),
        modulation(modulation)
    {}

    const IModulation *getModulation() const { return modulation; }
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_SIGNALBITMODEL_H
