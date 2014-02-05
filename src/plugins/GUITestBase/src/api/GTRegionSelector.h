/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_GT_REGIONSELECTOR_H_
#define _U2_GT_REGIONSELECTOR_H_

#include "api/GTGlobals.h"
#include <U2Gui/RegionSelector.h>

namespace U2 {

class GTRegionSelector {
public:
    class RegionSelectorSettings {
    public:
        RegionSelectorSettings(): start(0), end(0), unset(true){}
        RegionSelectorSettings(int _start, int _end): start(_start), end(_end), unset(false){}
        bool isUnset() const { return unset; }

        int start, end;
    private:
        bool unset;
    };

    static void setRegion(U2OpStatus& os, RegionSelector *regionSelector, const RegionSelectorSettings& s);
};

}
#endif // _U2_GT_REGIONSELECTOR_H_
