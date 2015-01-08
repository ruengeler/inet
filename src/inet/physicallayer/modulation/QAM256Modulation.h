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

#ifndef __INET_QAM256MODULATION_H
#define __INET_QAM256MODULATION_H

#include "inet/physicallayer/base/APSKModulationBase.h"

namespace inet {

namespace physicallayer {

class INET_API QAM256Modulation : public APSKModulationBase
{
    public:
        static const QAM256Modulation singleton;

    protected:
        static const APSKSymbol encodingTable[256];
        static const double kMOD;
        static const int m;

    public:
        QAM256Modulation();
        double calculateBER(double snir, double bandwidth, double bitrate) const;
        double calculateSER(double snir) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_QAM256MODULATION_H

