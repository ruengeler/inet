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

#include "inet/physicallayer/base/layered/LayeredErrorModelBase.h"
#include "inet/physicallayer/common/layered/SignalPacketModel.h"
#include "inet/physicallayer/common/layered/SignalBitModel.h"
#include "inet/physicallayer/common/layered/SignalSymbolModel.h"
#include "inet/physicallayer/common/layered/SignalSampleModel.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/apsk/layered/APSKSymbol.h"

namespace inet {

namespace physicallayer {

const IReceptionBitModel *LayeredErrorModelBase::computeBitModel(const LayeredTransmission *transmission, double bitErrorRate) const
{
    if (bitErrorRate == 0) {
        const TransmissionBitModel *transmissionBitModel = check_and_cast<const TransmissionBitModel *>(transmission->getBitModel());
        return new const ReceptionBitModel(transmissionBitModel->getHeaderBitLength(), transmissionBitModel->getPayloadBitLength(), transmissionBitModel->getHeaderBitRate(), transmissionBitModel->getPayloadBitRate(), transmissionBitModel->getBits());
    }
    else
        throw cRuntimeError("Not yet implemented");
}

const IReceptionSymbolModel *LayeredErrorModelBase::computeSymbolModel(const LayeredTransmission *transmission, double symbolErrorRate) const
{
    if (symbolErrorRate == 0) {
        const TransmissionSymbolModel *transmissionSymbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
        return new ReceptionSymbolModel(transmissionSymbolModel->getSymbolLength(), transmissionSymbolModel->getSymbolRate(), transmissionSymbolModel->getSymbols());
    }
    else {
        const TransmissionSymbolModel *transmissionSymbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
        const APSKModulationBase *modulation = check_and_cast<const APSKModulationBase *>(transmissionSymbolModel->getModulation());
        unsigned int constellationSize = modulation->getConstellationSize();
        const APSKSymbol *constellationDiagram = modulation->getEncodingTable();
        const std::vector<const ISymbol*> *transmittedSymbols = transmissionSymbolModel->getSymbols();
        std::vector<const ISymbol*> *receivedSymbols = new std::vector<const ISymbol *>(); // FIXME: memory leak
        for (unsigned int i = 0; i < transmittedSymbols->size(); i++) {
            if (uniform(0, 1) <= symbolErrorRate) {
                int receivedSymbolIndex = intuniform(0, constellationSize - 1); // TODO: it can be equal to the current symbol
                const APSKSymbol *receivedSymbol = &constellationDiagram[receivedSymbolIndex];
                receivedSymbols->push_back(receivedSymbol);
            }
            else
                receivedSymbols->push_back(transmittedSymbols->at(i));
        }
        return new ReceptionSymbolModel(transmissionSymbolModel->getSymbolLength(), transmissionSymbolModel->getSymbolRate(), receivedSymbols);
    }
}

} // namespace physicallayer

} // namespace inet

