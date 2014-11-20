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

#ifndef __INET_SIGNALPACKETMODEL_H
#define __INET_SIGNALPACKETMODEL_H

#include "inet/physicallayer/contract/ISignalPacketModel.h"

namespace inet {

namespace physicallayer {

class INET_API SignalPacketModel : public virtual ISignalPacketModel
{
  protected:
    const cPacket *packet;

  public:
    SignalPacketModel() :
        packet(NULL)
    {}

    SignalPacketModel(const cPacket *packet) :
        packet(packet)
    {}

    virtual void printToStream(std::ostream &stream) const;
    virtual const cPacket *getPacket() const { return packet; }
};

class INET_API TransmissionPacketModel : public SignalPacketModel, public virtual ITransmissionPacketModel
{
  public:
    TransmissionPacketModel() :
        SignalPacketModel()
    {}

    TransmissionPacketModel(const cPacket *packet) :
        SignalPacketModel(packet)
    {}
};

class INET_API ReceptionPacketModel : public SignalPacketModel, public IReceptionPacketModel
{
  protected:
    const BitVector *serializedPacket;
    const IForwardErrorCorrection *forwardErrorCorrection;
    const IScrambling *scrambling;
    const IInterleaving *interleaving;
    const double per;
    const bool packetErrorless;

  public:
    ReceptionPacketModel() :
        SignalPacketModel(),
        serializedPacket(NULL),
        forwardErrorCorrection(NULL),
        scrambling(NULL),
        interleaving(NULL),
        per(sNaN),
        packetErrorless(false)
    {}

    ReceptionPacketModel(const cPacket *packet, const BitVector *serializedPacket, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving, double per, bool packetErrorless) :
        SignalPacketModel(packet),
        serializedPacket(serializedPacket),
        forwardErrorCorrection(forwardErrorCorrection),
        scrambling(scrambling),
        interleaving(interleaving),
        per(per),
        packetErrorless(packetErrorless)
    {}

    virtual double getPER() const { return per; }
    virtual bool isPacketErrorless() const { return packetErrorless; }
    virtual const IForwardErrorCorrection *getForwardErrorCorrection() const { return forwardErrorCorrection; }
    virtual const IScrambling *getScrambling() const { return scrambling; }
    virtual const IInterleaving *getInterleaving() const { return interleaving; }
    virtual const BitVector *getSerializedPacket() const { return serializedPacket; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SIGNALPACKETMODEL_H
