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

#ifndef __IENT_IEEE80211LAYEREDERRORMODEL_H
#define __INET_IEEE80211LAYEREDERRORMODEL_H

#include "inet/physicallayer/layered/LayeredErrorModel.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211LayeredErrorModel : public LayeredErrorModel
{
    public:
        virtual const IReceptionPacketModel *computePacketModel(const LayeredTransmission *transmission, const ISNIR *snir) const;
        virtual const IReceptionBitModel *computeBitModel(const LayeredTransmission *transmission, const ISNIR *snir) const;
        virtual const IReceptionSymbolModel *computeSymbolModel(const LayeredTransmission *transmission, const ISNIR *snir) const;
        virtual const IReceptionSampleModel *computeSampleModel(const LayeredTransmission *transmission, const ISNIR *snir) const;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* IEEE80211LAYEREDERRORMODEL_H_ */
