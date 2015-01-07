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

#ifndef __INET_APSKSYMBOL_H
#define __INET_APSKSYMBOL_H

#include "inet/common/Complex.h"
#include "inet/physicallayer/contract/layered/ISymbol.h"

namespace inet {

namespace physicallayer {

class INET_API APSKSymbol : public Complex, public ISymbol
{
    public:
        APSKSymbol() : Complex() {}
        APSKSymbol(const double& r) : Complex(r) {}
        APSKSymbol(const double& q, const double& i) : Complex(q, i) {}
        APSKSymbol(const Complex& w) : Complex(w) {}
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_APSKSYMBOL_H

