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

#ifndef __INET_DUMMYSERIALIZER_H
#define __INET_DUMMYSERIALIZER_H

#include "inet/physicallayer/contract/layered/ISerializer.h"
#include "inet/common/BitVector.h"

namespace inet {
namespace physicallayer {

class INET_API DummySerializer : public cSimpleModule, public ISerializer
{
    protected:
        BitVector dummyOutputBits;
        cPacket *dummyOutputPacket;

    public:
        DummySerializer() : dummyOutputPacket(NULL) {};
        void setDummyOutputBits(const BitVector& bits) { this->dummyOutputBits = bits; }
        void setDummyOutputPacket(const cPacket *packet) { this->dummyOutputPacket = packet->dup(); }
        BitVector serialize(const cPacket *packet) const;
        cPacket *deserialize(const BitVector& bits) const;
        void printToStream(std::ostream &stream) const { stream << "A very dummy serializer"; }
        ~DummySerializer() { delete dummyOutputPacket; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_DUMMYSERIALIZER_H */
