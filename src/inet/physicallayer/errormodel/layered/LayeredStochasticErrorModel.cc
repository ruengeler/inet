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

#include "inet/physicallayer/errormodel/layered/LayeredStochasticErrorModel.h"
#include "inet/physicallayer/common/layered/SignalPacketModel.h"
#include "inet/physicallayer/common/layered/SignalBitModel.h"
#include "inet/physicallayer/common/layered/SignalSymbolModel.h"
#include "inet/physicallayer/common/layered/SignalSampleModel.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/apsk/layered/APSKSymbol.h"

namespace inet {

namespace physicallayer {

Define_Module(LayeredStochasticErrorModel);

LayeredStochasticErrorModel::LayeredStochasticErrorModel() :
    packetErrorRate(NaN),
    bitErrorRate(NaN),
    symbolErrorRate(NaN)
{
}

void LayeredStochasticErrorModel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        packetErrorRate = par("packetErrorRate");
        bitErrorRate = par("bitErrorRate");
        symbolErrorRate = par("symbolErrorRate");
    }
}

void LayeredStochasticErrorModel::printToStream(std::ostream& stream) const
{
    stream << "LayeredStochasticErrorModel, "
           << "packetErrorRate = " << packetErrorRate << ", "
           << "bitErrorRate = " << bitErrorRate << ", "
           << "symbolErrorRate = " << symbolErrorRate;
}

const IReceptionPacketModel *LayeredStochasticErrorModel::computePacketModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    throw cRuntimeError("Not yet implemented");
}

const IReceptionBitModel *LayeredStochasticErrorModel::computeBitModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    return LayeredErrorModelBase::computeBitModel(transmission, bitErrorRate);
}

const IReceptionSymbolModel *LayeredStochasticErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    return LayeredErrorModelBase::computeSymbolModel(transmission, symbolErrorRate);
}

const IReceptionSampleModel *LayeredStochasticErrorModel::computeSampleModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    throw cRuntimeError("Not yet implemented");
}

} // namespace physicallayer

} // namespace inet

