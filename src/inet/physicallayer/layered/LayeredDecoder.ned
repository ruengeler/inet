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

package inet.physicallayer.layered;

import inet.physicallayer.contract.IDecoder;
import inet.physicallayer.contract.IFECCoder;
import inet.physicallayer.contract.IInterleaver;
import inet.physicallayer.contract.IScrambler;
import inet.physicallayer.contract.ISerializer;


module LayeredDecoder like IDecoder
{
    parameters:
        string fecType = default("");
        string deinterleaverType = default("");
        string descramblerType = default("");
        @class(LayeredDecoder);

    submodules:
        descrambler: <descramblerType> like IScrambler if descramblerType != "" {
            @display("p=72,94");
        }
        fecDecoder: <fecType> like IFECCoder if fecType != "" {
            @display("p=72,168");
        }
        deinterleaverType: <deinterleaverType> like IInterleaver if deinterleaverType != "" {
            @display("p=72,244");
        }
}
