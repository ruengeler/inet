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

#ifndef __INET_LAYEREDERRORMODEL_H
#define __INET_LAYEREDERRORMODEL_H

#include "inet/physicallayer/contract/IErrorModel.h"

namespace inet {
namespace physicallayer {

class INET_API LayeredErrorModel : public ILayeredErrorModel, public cSimpleModule
{
    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("The module doesn't handle self messages"); }
        void corruptBits(BitVector *bits, double ber, int begin, int end) const;

    public:
        virtual const IReceptionPacketModel *computePacketModel(const LayeredTransmission *transmission, const ISNIR *snir) const = 0;
        virtual const IReceptionBitModel *computeBitModel(const LayeredTransmission *transmission, const ISNIR *snir) const = 0;
        virtual const IReceptionSymbolModel *computeSymbolModel(const LayeredTransmission *transmission, const ISNIR *snir) const = 0;
        virtual const IReceptionSampleModel *computeSampleModel(const LayeredTransmission *transmission, const ISNIR *snir) const = 0;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_LAYEREDERRORMODEL_H */
