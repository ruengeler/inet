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

#ifndef __INET_APSKDECODERMODULE_H
#define __INET_APSKDECODERMODULE_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/contract/layered/IDecoder.h"
#include "inet/physicallayer/apsk/layered/APSKDecoder.h"
#include "inet/physicallayer/common/ConvolutionalCoder.h"

namespace inet {
namespace physicallayer {

class INET_API APSKDecoderModule : public cSimpleModule, public IDecoder
{
    protected:
        const APSKDecoder *layeredDecoder;
        const IScrambler *descrambler;
        const IFECCoder *fecDecoder;
        const IInterleaver *deinterleaver;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }

    public:
        virtual void printToStream(std::ostream& stream) const { stream << "APSKDecoder"; }
        const APSKCode *getCode() const { return layeredDecoder->getCode(); }
        const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const;
        virtual ~APSKDecoderModule();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_APSKDECODERMODULE_H */
