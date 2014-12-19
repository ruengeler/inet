//
// Copyright (C) 2014 OpenSim Ltd
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

package inet.physicallayer.apsk.layered;

import inet.physicallayer.contract.IDigitalAnalogConverter;
import inet.physicallayer.contract.IEncoder;
import inet.physicallayer.contract.IModulator;
import inet.physicallayer.contract.IPulseShaper;
import inet.physicallayer.contract.ITransmitter;


module APSKTransmitter like ITransmitter
{
    parameters:
        string levelOfDetail @enum("bit","symbol","sample") = default("symbol");
        string signalEncoderType = default("");
        string encoderType = default("");
        string signalModulatorType = default("");
        string modulatorType = default("");
        string pulseShaperType = default("");
        string digitalAnalogConverterType = default("");
        double bitrate @unit(bps);
        double power @unit(W);
        double carrierFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        @class(APSKTransmitter);

    submodules:
        signalEncoder: <signalEncoderType> like IEncoder if signalEncoderType != "" {
            @display("p=100,50");
        }
        encoder: <encoderType> like IEncoder if encoderType != "" {
            @display("p=207,50");
        }
        modulator: <modulatorType> like IModulator if modulatorType != "" {
            @display("p=100,150");
        }
        signalModulator: <signalModulatorType> like IModulator if signalModulatorType != "" {
            @display("p=207,150");
        }
        pulseShaper: <pulseShaperType> like IPulseShaper if pulseShaperType != "" {
            @display("p=100,250");
        }
        digitalAnalogConverter: <digitalAnalogConverterType> like IDigitalAnalogConverter if digitalAnalogConverterType != "" {
            @display("p=100,350");
        }
}
