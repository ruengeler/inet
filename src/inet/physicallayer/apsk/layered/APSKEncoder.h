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

#ifndef __INET_APSKENCODER_H
#define __INET_APSKENCODER_H

#include "inet/physicallayer/contract/layered/IEncoder.h"
#include "inet/physicallayer/contract/layered/ISerializer.h"
#include "inet/physicallayer/contract/layered/IFECCoder.h"
#include "inet/physicallayer/contract/layered/IScrambler.h"
#include "inet/physicallayer/contract/layered/IInterleaver.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/layered/SignalBitModel.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/apsk/APSKCode.h"

namespace inet {
namespace physicallayer {

class INET_API APSKEncoder : public IEncoder
{
    protected:
        const IFECCoder *fecEncoder;
        const IInterleaver *interleaver;
        const IScrambler *scrambler;
        const APSKCode *code;

    public:
        virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const;
        virtual void printToStream(std::ostream& stream) const { stream << "APSK Layered Encoder"; }
        const APSKCode *getCode() const { return code; }
        APSKEncoder();
        APSKEncoder(const APSKCode *code);
        APSKEncoder(const IFECCoder *fecEncoder, const IInterleaver *interleaver, const IScrambler *scrambler);
        ~APSKEncoder();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_APSKENCODER_H */
