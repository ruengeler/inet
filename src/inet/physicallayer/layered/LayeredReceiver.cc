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

#include "inet/physicallayer/contract/ISNIR.h"
#include "inet/physicallayer/contract/IErrorModel.h"
#include "inet/physicallayer/common/ReceptionDecision.h"
#include "inet/physicallayer/layered/LayeredReceiver.h"
#include "inet/physicallayer/layered/LayeredTransmission.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/layered/LayeredReception.h"
#include "inet/physicallayer/layered/SignalSymbolModel.h"
#include "inet/physicallayer/layered/SignalAnalogModel.h"
#include "inet/physicallayer/layered/SignalBitModel.h"
#include "inet/physicallayer/layered/LayeredSNIR.h"

namespace inet {

namespace physicallayer {

LayeredReceiver::LayeredReceiver() :
    decoder(NULL),
    demodulator(NULL),
    pulseFilter(NULL),
    analogDigitalConverter(NULL)
{
}

void LayeredReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        errorModel = check_and_cast<ILayeredErrorModel *>(getSubmodule("errorModel"));
        decoder = check_and_cast<IDecoder *>(getSubmodule("decoder"));
        demodulator = check_and_cast<IDemodulator *>(getSubmodule("demodulator"));
        pulseFilter = check_and_cast<IPulseFilter *>(getSubmodule("pulseFilter"));
        analogDigitalConverter = check_and_cast<IAnalogDigitalConverter *>(getSubmodule("analogDigitalConverter"));

        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
    }
}

const IReceptionSymbolModel* LayeredReceiver::createReceptionSymbolModel(const ITransmissionSymbolModel* symbolModel) const
{
    const TransmissionSymbolModel *transmissionSymbolModel = dynamic_cast<const TransmissionSymbolModel*>(symbolModel);
    return new ReceptionSymbolModel(transmissionSymbolModel->getSymbolLength(), transmissionSymbolModel->getSymbolRate(), transmissionSymbolModel->getSymbols(), 0, 0);
}

const IReceptionAnalogModel* LayeredReceiver::createReceptionAnalogModel(const ITransmissionAnalogModel* analogModel, const IReception *reception, const INoise *noise) const
{
    const ISNIR *snir = computeSNIR(reception, analogModel, noise);
}

const ISNIR* LayeredReceiver::computeSNIR(const IReception* reception, const ITransmissionAnalogModel* analogModel, const INoise* noise) const
{
    const LayeredReception *layeredReception = check_and_cast<const LayeredReception *>(reception);
    const ScalarNoise *scalarNoise = check_and_cast<const ScalarNoise *>(noise);
    return new LayeredSNIR(layeredReception, scalarNoise);
}

//const IListening *LayeredReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
//{
//    return new BandListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, bandwidth);
//}
//
//const IListeningDecision *LayeredReceiver::computeListeningDecision(const IListening *listening, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
//{
//    const INoise *noise = computeNoise(listening, interferingReceptions, backgroundNoise);
//    const FlatNoiseBase *flatNoise = check_and_cast<const FlatNoiseBase *>(noise);
//    W maxPower = flatNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
//    delete noise;
//    return new ListeningDecision(listening, maxPower >= energyDetection);
//}

const IReceptionDecision *LayeredReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
{
    const LayeredTransmission *transmission = dynamic_cast<const LayeredTransmission*>(reception->getTransmission());
    const LayeredReception *layeredReception = dynamic_cast<const LayeredReception*>(reception);
    const ITransmissionAnalogModel *analogModel = transmission->getAnalogModel();
    const ITransmissionSampleModel *sampleModel = transmission->getSampleModel();
    const ITransmissionSymbolModel *symbolModel = transmission->getSymbolModel();
    const ITransmissionPacketModel *packetModel = transmission->getPacketModel();
    const IReceptionSampleModel *receptionSampleModel = NULL;
//    const INoise *noise = computeNoise(listening, interferingReceptions);
    const INoise *noise = NULL;
    bool isReceptionAttempted = true;
    bool isReceptionPossible = computeIsReceptionPossible(listening, reception);
    // bool isReceptionSuccessful = isReceptionPossible && snirMin > snirThreshold;
    // const IReceptionAnalogModel *analogModel = NULL; // receptionXXX->getAnalogModel();
    const IReceptionAnalogModel *receptionAnalogModel = createReceptionAnalogModel(analogModel, reception, noise);
    // const IReceptionSampleModel *sampleModel = analogDigitalConverter->convertAnalogToDigital(analogModel);
    const IReceptionSymbolModel *receptionSymbolModel = createReceptionSymbolModel(symbolModel);
    const IReceptionBitModel *bitModel = demodulator->demodulate(receptionSymbolModel);
    const IReceptionPacketModel *receptionPacketModel = decoder->decode(bitModel);
    const cPacket *macFrame = transmission->getMacFrame();
    const IReceptionPacketModel *hackedPacketModel = new ReceptionPacketModel(macFrame, receptionPacketModel->getForwardErrorCorrection(),
                                                                              receptionPacketModel->getScrambling(), receptionPacketModel->getInterleaving(),
                                                                              receptionPacketModel->getPER(), receptionPacketModel->isPacketErrorless());
    return new ReceptionDecision(reception, NULL, hackedPacketModel, isReceptionPossible, isReceptionAttempted, true); // isReceptionSuccessful
}

} // namespace physicallayer

} // namespace inet
