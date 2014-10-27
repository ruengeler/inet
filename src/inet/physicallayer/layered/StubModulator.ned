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

import inet.physicallayer.contract.IModulator;

//
// This module, in fact, doesn't modulate its input. It creates
// a symbol model that contains the modulation scheme metadata
// without the actual symbol representation of its input.
//
// This module is necessary for the error model since BER
// calculation requires a primitive symbol model that contains
// at least the modulation scheme metadata.
//
// This can be useful if no modulator implementation is present
// (for a modulation scheme) or computing the symbols would be too
// costly.
//
simple StubModulator like IModulator
{
    parameters:
        string modulationScheme; // arbitrary
        @class(StubModulator);
}
