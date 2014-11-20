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
#include "inet/physicallayer/contract/ISymbol.h"
#include "inet/physicallayer/layered/LayeredReceiver.h"
#include "inet/physicallayer/layered/LayeredScalarTransmission.h"
#include "inet/physicallayer/layered/LayeredReceptionDecision.h"
#include "inet/physicallayer/layered/LayeredScalarReception.h"
#include "inet/physicallayer/layered/SignalSymbolModel.h"
#include "inet/physicallayer/layered/SignalSampleModel.h"
#include "inet/physicallayer/analogmodel/layered/SignalAnalogModel.h"
#include "inet/physicallayer/common/BandListening.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211LayeredDecoder.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMDemodulator.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"

namespace inet {
namespace physicallayer {

#define OFDM_SYMBOL_SIZE 48

Define_Module(Ieee80211LayeredReceiver);

void Ieee80211LayeredReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        errorModel = dynamic_cast<ILayeredErrorModel *>(getSubmodule("errorModel"));
        decoder = dynamic_cast<IDecoder *>(getSubmodule("decoder"));
        headerDecoder = dynamic_cast<IDecoder *>(getSubmodule("headerDecoder"));
        demodulator = dynamic_cast<IDemodulator *>(getSubmodule("demodulator"));
        headerDemodulator = dynamic_cast<IDemodulator *>(getSubmodule("headerDemodulator"));
        pulseFilter = dynamic_cast<IPulseFilter *>(getSubmodule("pulseFilter"));
        analogDigitalConverter = dynamic_cast<IAnalogDigitalConverter *>(getSubmodule("analogDigitalConverter"));

        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        // TODO: temporary parameters
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
        snirThreshold = math::dB2fraction(par("snirThreshold"));
        channelSpacing = Hz(par("channelSpacing"));
    }
}

const IReceptionSymbolModel* Ieee80211LayeredReceiver::createSignalFieldReceptionSymbolModel(const IReceptionSymbolModel* receptionSymbolModel) const
{
//    return new ReceptionSymbolModel(transmissionSymbolModel->getSymbolLength(), transmissionSymbolModel->getSymbolRate(), corruptedSymbols);
    const std::vector<const ISymbol *> *symbols = receptionSymbolModel->getSymbols();
    std::vector<const ISymbol *> *signalSymbols = new std::vector<const ISymbol *>(); // FIXME: memory leak
    signalSymbols->push_back(symbols->at(0)); // The first symbol is the signal field symbol
    // TODO: revise symbolLength, symbolRate  !!
    return new ReceptionSymbolModel(1, receptionSymbolModel->getSymbolRate(), signalSymbols);
}

const IReceptionPacketModel *Ieee80211LayeredReceiver::demodulateAndDecodeSignalField(const IRadioMedium *medium, const IRadio *receiver, const LayeredScalarTransmission *transmission, const IReceptionSymbolModel *receptionSymbolModel) const
{
    const IReceptionSymbolModel *signalFieldReceptionSymbolModel = NULL;
    const IReceptionBitModel *signalFieldReceptionBitModel = NULL;
    const IReceptionPacketModel *signalFieldReceptionPacketModel = NULL;
    if (levelOfDetail >= SYMBOL_DOMAIN)
    {
        if (!receptionSymbolModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            ASSERT(transmission->getSymbolModel() != NULL);
            receptionSymbolModel = errorModel->computeSymbolModel(transmission, snir);
            signalFieldReceptionSymbolModel = createSignalFieldReceptionSymbolModel(receptionSymbolModel);
        }
        if (headerDemodulator) // non-compliant mode
            signalFieldReceptionBitModel = headerDemodulator->demodulate(signalFieldReceptionSymbolModel);
        else
        {
            // In compliant mode, the signal field modulation is always BPSK
            const Ieee80211OFDMDemodulator demodulator(&BPSKModulation::singleton);
            signalFieldReceptionBitModel = demodulator.demodulate(signalFieldReceptionSymbolModel);
        }
    }
    if (levelOfDetail >= BIT_DOMAIN)
    {
        if (!signalFieldReceptionBitModel)
        {
            const ISNIR *snir = medium->getSNIR(receiver, transmission);
            ASSERT(transmission->getBitModel() != NULL);
            signalFieldReceptionBitModel = errorModel->computeBitModel(transmission, snir);
        }
        if (headerDecoder) // non-compliant mode
            signalFieldReceptionPacketModel = decoder->decode(signalFieldReceptionBitModel);
        else
        {
            // In compliant mode, the default decoder parameters are as follows
            const Ieee80211LayeredDecoder decoder(new Ieee80211Scrambler(new Ieee80211Scrambling("1011101", "0001001")),
                                            new ConvolutionalCoder(new Ieee80211ConvolutionalCode(1,2)),
                                            new Ieee80211Interleaver(new Ieee80211Interleaving(BPSKModulation::singleton.getCodeWordLength() * OFDM_SYMBOL_SIZE, BPSKModulation::singleton.getCodeWordLength())));
            signalFieldReceptionPacketModel = decoder.decode(signalFieldReceptionBitModel);
        }
    }
    return signalFieldReceptionPacketModel;
}

const IReceptionDecision *Ieee80211LayeredReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const IRadio *receiver = reception->getReceiver();
    const IRadioMedium *medium = receiver->getMedium();
    const LayeredScalarTransmission *transmission = dynamic_cast<const LayeredScalarTransmission*>(reception->getTransmission());
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
    const IReceptionPacketModel *signalFieldReceptionPacketModel = demodulateAndDecodeSignalField(medium, receiver, transmission, receptionSymbolModel);
    if (headerDemodulator)
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
        receptionBitModel = headerDemodulator->demodulate(receptionSymbolModel);
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
    const ReceptionPacketModel *hackedPacketModel = new ReceptionPacketModel(macFrame, receptionPacketModel->getSerializedPacket(), receptionPacketModel->getForwardErrorCorrection(),
                                                                              receptionPacketModel->getScrambling(), receptionPacketModel->getInterleaving(),
                                                                              receptionPacketModel->getPER(), receptionPacketModel->isPacketErrorless());
    return new LayeredReceptionDecision(reception, receptionIndication, hackedPacketModel, NULL, NULL, NULL, NULL, true, true, hackedPacketModel->isPacketErrorless());
}

} /* namespace physicallayer */
} /* namespace inet */
