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
#include "inet/physicallayer/layered/SignalSampleModel.h"
#include "inet/physicallayer/layered/SignalAnalogModel.h"
#include "inet/physicallayer/layered/SignalBitModel.h"
#include "inet/physicallayer/layered/LayeredSNIR.h"
#include "inet/physicallayer/common/BandListening.h"

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
        // TODO: temporary parameters
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

const IReceptionSampleModel* LayeredReceiver::createReceptionSampleModel(const ITransmissionSampleModel* sampleModel) const
{
    throw cRuntimeError("Unimplemented");
    int sampleLength = sampleModel->getSampleLength();
    double sampleRate = sampleModel->getSampleRate();
    const std::vector<W> *samples = sampleModel->getSamples();
    W rssi = W(0); // TODO: error model
    return new const ReceptionSampleModel(sampleLength, sampleRate, samples, rssi);
}

const IReceptionBitModel* LayeredReceiver::createReceptionBitModel(const ITransmissionBitModel* bitModel) const
{
    throw cRuntimeError("Unimplemented");
    int bitLength = bitModel->getBitLength();
    double bitRate = bitModel->getBitRate();
    const BitVector *bits = bitModel->getBits();
    const IModulation *modulation = NULL; // TODO: TransmissionBitModel doesn't include this info
    double ber = -1; // TODO: ber, bitErrorCount -> error module should provide some statistical
    int bitErrorCount = -1;
    return new const ReceptionBitModel(bitLength, bitRate, bits, modulation, ber, bitErrorCount);
}


const IReceptionPacketModel* LayeredReceiver::createReceptionPacketModel(const ITransmissionPacketModel* packetModel) const
{

}


const ISNIR* LayeredReceiver::computeSNIR(const IReception *reception, const IListening *listening, const IInterference *interference) const
{
    const LayeredReception *layeredReception = check_and_cast<const LayeredReception *>(reception);
    const INoise *noise = computeNoise(listening, interference);
    const ScalarNoise *scalarNoise = check_and_cast<const ScalarNoise *>(noise);
    return new LayeredSNIR(layeredReception, scalarNoise);
}

const INoise *LayeredReceiver::computeNoise(const IListening *listening, const IInterference *interference) const
{
    // FIXME: copied from ScalarReceiver
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (std::vector<const IReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++) {
        const LayeredReception *reception = check_and_cast<const LayeredReception *>(*it);
        if (carrierFrequency == reception->getCarrierFrequency() && bandwidth == reception->getBandwidth()) {
            W power = reception->getPower();
            simtime_t startTime = reception->getStartTime();
            simtime_t endTime = reception->getEndTime();
            if (startTime < noiseStartTime)
                noiseStartTime = startTime;
            if (endTime > noiseEndTime)
                noiseEndTime = endTime;
            std::map<simtime_t, W>::iterator itStartTime = powerChanges->find(startTime);
            if (itStartTime != powerChanges->end())
                itStartTime->second += power;
            else
                powerChanges->insert(std::pair<simtime_t, W>(startTime, power));
            std::map<simtime_t, W>::iterator itEndTime = powerChanges->find(endTime);
            if (itEndTime != powerChanges->end())
                itEndTime->second -= power;
            else
                powerChanges->insert(std::pair<simtime_t, W>(endTime, -power));
        }
        else if (areOverlappingBands(carrierFrequency, bandwidth, reception->getCarrierFrequency(), reception->getBandwidth()))
            throw cRuntimeError("Overlapping bands are not supported");
    }
    const ScalarNoise *scalarBackgroundNoise = dynamic_cast<const ScalarNoise *>(interference->getBackgroundNoise());
    if (scalarBackgroundNoise) {
        if (carrierFrequency == scalarBackgroundNoise->getCarrierFrequency() && bandwidth == scalarBackgroundNoise->getBandwidth()) {
            const std::map<simtime_t, W> *backgroundNoisePowerChanges = scalarBackgroundNoise->getPowerChanges();
            for (std::map<simtime_t, W>::const_iterator it = backgroundNoisePowerChanges->begin(); it != backgroundNoisePowerChanges->end(); it++) {
                std::map<simtime_t, W>::iterator jt = powerChanges->find(it->first);
                if (jt != powerChanges->end())
                    jt->second += it->second;
                else
                    powerChanges->insert(std::pair<simtime_t, W>(it->first, it->second));
            }
        }
        else if (areOverlappingBands(carrierFrequency, bandwidth, scalarBackgroundNoise->getCarrierFrequency(), scalarBackgroundNoise->getBandwidth()))
            throw cRuntimeError("Overlapping bands are not supported");
    }
    return new ScalarNoise(noiseStartTime, noiseEndTime, carrierFrequency, bandwidth, powerChanges);
}

const IReceptionDecision *LayeredReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
//    const LayeredTransmission *transmission = dynamic_cast<const LayeredTransmission*>(reception->getTransmission());
//    const IReceptionAnalogModel *receptionAnalogModel = NULL;
//    const IReceptionSampleModel *receptionSampleModel = NULL;
//    const IReceptionSymbolModel *receptionSymbolModel = NULL;
//    const IReceptionBitModel *receptionBitModel = NULL;
//    const IReceptionPacketModel *receptionPacketModel = NULL;
//    if (analogDigitalConverter)
//    {
//        const IReceptionAnalogModel *totalAnalogModel = NULL; // TODO: interference + receptionAnalogModel;
//        receptionSampleModel = analogDigitalConverter->convertAnalogToDigital(totalAnalogModel);
//    }
//    if (pulseFilter)
//    {
//        if (!receptionSampleModel)
//        {
//            const ISNIR *snir = computeSNIR(reception, listening, interference);
//            receptionSampleModel = errorModel->computeSampleModel(transmission->getSampleModel(), snir);
//        }
//        receptionSymbolModel = pulseFilter->filter(receptionSampleModel);
//    }
//    if (demodulator)
//    {
//        if (!receptionSymbolModel)
//        {
//            const ISNIR *snir = computeSNIR(reception, listening, interference);
//            receptionSymbolModel = errorModel->computeSymbolModel(transmission->getSymbolModel(), snir);
//        }
//        receptionBitModel = demodulator->demodulate(receptionSymbolModel);
//    }
//    if (decoder)
//    {
//        if (!receptionBitModel)
//        {
//            const ISNIR *snir = computeSNIR(reception, listening, interference);
//            receptionBitModel = errorModel->computeBitModel(transmission->getBitModel(), snir);
//        }
//        receptionPacketModel = decoder->decode(receptionBitModel);
//    }
//    if (!receptionPacketModel)
//        throw


    const LayeredTransmission *transmission = dynamic_cast<const LayeredTransmission*>(reception->getTransmission());
    const LayeredReception *layeredReception = dynamic_cast<const LayeredReception*>(reception);
    const IReceptionAnalogModel *receptionAnalogModel = layeredReception->getAnalogModel();
    ASSERT(receptionAnalogModel != NULL); // analog model is obligatory
    const IReceptionSampleModel *receptionSampleModel = NULL;
    if (analogDigitalConverter)
    {
//        TODO: calculate totalAnalogModel -> convertAnalogToDigital(totalAnalogModel)
        receptionSampleModel = analogDigitalConverter->convertAnalogToDigital(receptionAnalogModel);
    }
    else
    {
        const ITransmissionSampleModel *sampleModel = transmission->getSampleModel();
        if (sampleModel)
            receptionSampleModel = createReceptionSampleModel(sampleModel);
    }
    const IReceptionSymbolModel *receptionSymbolModel = NULL;
    if (pulseFilter && receptionSampleModel)
        receptionSymbolModel = pulseFilter->filter(receptionSampleModel);
    else if (pulseFilter && !receptionSampleModel)
        throw cRuntimeError("Pulse filter is present but there is no sample model");
    else
    {
        const ITransmissionSymbolModel *symbolModel = transmission->getSymbolModel();
        if (symbolModel)
        {
            const ISNIR *snir = computeSNIR(reception, listening, interference);
            receptionSymbolModel = errorModel->computeSymbolModel(symbolModel, snir);
        }
    }
    const IReceptionBitModel *receptionBitModel = NULL;
    if (demodulator && receptionSymbolModel)
        receptionBitModel = demodulator->demodulate(receptionSymbolModel);
    else if (demodulator && !receptionSymbolModel)
        throw cRuntimeError("Demodulator is present but there is no symbol model");
    else
    {
        const ITransmissionBitModel *bitModel = transmission->getBitModel();
        if (bitModel)
        {
            const ISNIR *snir = computeSNIR(reception, listening, interference);
            receptionBitModel = errorModel->computeBitModel(bitModel, snir);
        }
    }
    const IReceptionPacketModel *receptionPacketModel = NULL;
    if (decoder && receptionBitModel)
        receptionPacketModel = decoder->decode(receptionBitModel);
    else if (decoder && !receptionBitModel)
        throw cRuntimeError("Decoder is present but there is not bit representation");
    else
    {
        const ITransmissionPacketModel *packetModel = transmission->getPacketModel();
        if (packetModel)
        {
            const ISNIR *snir = computeSNIR(reception, listening, interference);
            receptionPacketModel = errorModel->computePacketModel(packetModel, snir);
        }
        else
            throw cRuntimeError("Packet model is obligatory");
    }
    // FIXME: !!HACK!!
    const cPacket *macFrame = transmission->getMacFrame();
    const ReceptionPacketModel *hackedPacketModel = new ReceptionPacketModel(macFrame, receptionPacketModel->getForwardErrorCorrection(),
                                                                              receptionPacketModel->getScrambling(), receptionPacketModel->getInterleaving(),
                                                                              receptionPacketModel->getPER(), receptionPacketModel->isPacketErrorless());
    // FIXME: !!HACK!!
    bool isReceptionAttempted = true;
    bool isReceptionPossible = computeIsReceptionPossible(listening, reception);
//    double snirMin = totalAnalogModel ...;
    double snirMin = 0; // TODO
    bool isReceptionSuccessful = isReceptionPossible && snirMin > snirThreshold;
    return new ReceptionDecision(reception, NULL, hackedPacketModel, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful);
}

} // namespace physicallayer

} // namespace inet
