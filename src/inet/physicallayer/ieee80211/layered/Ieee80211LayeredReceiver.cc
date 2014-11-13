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

#include "inet/physicallayer/ieee80211/layered/Ieee80211LayeredReceiver.h"
#include "inet/physicallayer/contract/ISNIR.h"
#include "inet/physicallayer/contract/IErrorModel.h"
#include "inet/physicallayer/layered/LayeredReceiver.h"
#include "inet/physicallayer/layered/LayeredTransmission.h"
#include "inet/physicallayer/layered/LayeredReceptionDecision.h"
#include "inet/physicallayer/layered/LayeredScalarReception.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/layered/SignalSymbolModel.h"
#include "inet/physicallayer/layered/SignalSampleModel.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"
#include "inet/physicallayer/common/BandListening.h"
#include "inet/physicallayer/contract/IRadioMedium.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211LayeredReceiver);

const IReceptionDecision *Ieee80211LayeredReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const IRadio *receiver = reception->getReceiver();
    const IRadioMedium *medium = receiver->getMedium();
    const LayeredTransmission *transmission = dynamic_cast<const LayeredTransmission*>(reception->getTransmission());
    const LayeredScalarReception *layeredReception = dynamic_cast<const LayeredScalarReception*>(reception);
    const IReceptionAnalogModel *receptionAnalogModel = layeredReception->getAnalogModel();
    if (!receptionAnalogModel)
        throw cRuntimeError("Reception analog model is obligatory");
    const IReceptionSampleModel *receptionSampleModel = NULL;
    const IReceptionSymbolModel *receptionSymbolModel = NULL;
    const IReceptionBitModel *receptionBitModel = NULL;
    const IReceptionPacketModel *receptionPacketModel = NULL;
    RadioReceptionIndication *receptionIndication = new RadioReceptionIndication();
    if (analogDigitalConverter)
    {
//        const IReceptionAnalogModel *totalAnalogModel = NULL; // TODO: interference + receptionAnalogModel;
//        receptionSampleModel = analogDigitalConverter->convertAnalogToDigital(receptionAnalogModel, snir);
    }
    if (pulseFilter)
    {
        if (!receptionSampleModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            receptionIndication->setMinSNIR(snir->getMin());
            ASSERT(transmission->getSampleModel() != NULL);
            receptionSampleModel = errorModel->computeSampleModel(transmission, snir);
        }
        receptionIndication->setMinRSSI(receptionSampleModel->getRSSI());
        receptionSymbolModel = pulseFilter->filter(receptionSampleModel);
    }
    if (demodulator)
    {
        if (!receptionSymbolModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            ASSERT(transmission->getSymbolModel() != NULL);
            receptionSymbolModel = errorModel->computeSymbolModel(transmission, snir);
        }
//        FIXME: delete ser from reception indication?
//        receptionIndication->setSymbolErrorCount(receptionSymbolModel->getSymbolErrorCount());
//        receptionIndication->setSymbolErrorRate(receptionSymbolModel->getSER());
        receptionBitModel = demodulator->demodulate(receptionSymbolModel);
    }
    if (decoder)
    {
        if (!receptionBitModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            ASSERT(transmission->getBitModel() != NULL);
            receptionBitModel = errorModel->computeBitModel(transmission, snir);
        }
//        FIXME: delete ber from reception indication?
//        receptionIndication->setBitErrorCount(receptionBitModel->getBitErrorCount());
//        receptionIndication->setBitErrorRate(receptionBitModel->getBER());
        receptionPacketModel = decoder->decode(receptionBitModel);
    }
    if (!receptionPacketModel)
        throw cRuntimeError("Packet model is obligatory");
    receptionIndication->setPacketErrorRate(receptionPacketModel->getPER());
    // FIXME: Kludge: we have no serializer yet
    const cPacket *macFrame = transmission->getMacFrame();
    const ReceptionPacketModel *hackedPacketModel = new ReceptionPacketModel(macFrame, receptionPacketModel->getForwardErrorCorrection(),
                                                                              receptionPacketModel->getScrambling(), receptionPacketModel->getInterleaving(),
                                                                              receptionPacketModel->getPER(), receptionPacketModel->isPacketErrorless());
    return new LayeredReceptionDecision(reception, receptionIndication, hackedPacketModel, NULL, NULL, NULL, NULL, true, true, hackedPacketModel->isPacketErrorless());
}

} /* namespace physicallayer */
} /* namespace inet */
