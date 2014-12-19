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

#include "inet/physicallayer/layered/LayeredReceptionDecision.h"

namespace inet {
namespace physicallayer {

LayeredReceptionDecision::LayeredReceptionDecision(
        const IReception* reception, const RadioReceptionIndication* indication,
        const IReceptionPacketModel* packetModel,
        const IReceptionBitModel* bitModel,
        const IReceptionSymbolModel* symbolModel,
        const IReceptionSampleModel* sampleModel,
        const IReceptionAnalogModel* analogModel, bool isReceptionPossible,
        bool isReceptionAttempted, bool isReceptionSuccessful) :
                ReceptionDecision(reception, indication, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful),
                bitModel(bitModel),
                symbolModel(symbolModel),
                sampleModel(sampleModel),
                analogModel(analogModel)
{
}

const cPacket *LayeredReceptionDecision::getMacFrame() const
{
    return packetModel->getPacket();
}

} /* namespace physicallayer */
} /* namespace inet */

