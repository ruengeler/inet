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

#include "inet/physicallayer/errormodel/layered/LayeredAPSKErrorModel.h"
#include "inet/physicallayer/common/layered/SignalPacketModel.h"
#include "inet/physicallayer/common/layered/SignalBitModel.h"
#include "inet/physicallayer/common/layered/SignalSymbolModel.h"
#include "inet/physicallayer/common/layered/SignalSampleModel.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/apsk/layered/APSKSymbol.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"
#include "inet/physicallayer/apsk/layered/APSKLayeredTransmission.h"
#include "inet/physicallayer/apsk/layered/APSKRadioFrame_m.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"

namespace inet {

namespace physicallayer {

Define_Module(LayeredAPSKErrorModel);

LayeredAPSKErrorModel::LayeredAPSKErrorModel()
{
}

const IReceptionPacketModel *LayeredAPSKErrorModel::computePacketModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    const ITransmissionPacketModel *packetModel = transmission->getPacketModel();
    const IReceptionPacketModel *receptionPacketModel = nullptr;
    const APSKRadioFrame *apskRadioFrame = check_and_cast<const APSKRadioFrame *>(packetModel->getPacket());
    int bitLength = apskRadioFrame->getBitLength();
    // TODO: incomplete
    return nullptr;
}
const IReceptionBitModel *LayeredAPSKErrorModel::computeBitModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    const ITransmissionBitModel* bitModel = transmission->getBitModel();
    const ITransmissionSymbolModel* symbolModel = transmission->getSymbolModel();
    const ScalarTransmissionSignalAnalogModel *analogModel = check_and_cast<const ScalarTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
    const APSKLayeredTransmission *apskLayeredTransmission = check_and_cast<const APSKLayeredTransmission *>(transmission);
    const IModulation* modulation = apskLayeredTransmission->getModulation();
    double bitErrorRate = modulation->calculateBER(snir->getMin(), analogModel->getBandwidth().get(), bitModel->getPayloadBitRate());
    return LayeredErrorModelBase::computeBitModel(transmission, bitErrorRate);
}

const IReceptionSymbolModel *LayeredAPSKErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    const IModulation* modulation = transmission->getSymbolModel()->getModulation();
    const ScalarTransmissionSignalAnalogModel *transmissionAnalogModel = dynamic_cast<const ScalarTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
    const ITransmissionBitModel* bitModel = transmission->getBitModel();
    double bandwidth = transmissionAnalogModel->getBandwidth().get();
    double symbolErrorRate = modulation->calculateSER(snir->getMin(), bandwidth, bitModel->getPayloadBitRate());
    return LayeredErrorModelBase::computeSymbolModel(transmission, symbolErrorRate);
}

const IReceptionSampleModel *LayeredAPSKErrorModel::computeSampleModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    throw cRuntimeError("Not yet implemented");
}

} // namespace physicallayer

} // namespace inet

