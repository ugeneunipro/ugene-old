/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GT_RUNNABLES_SMITH_WATERMAN_DIALOG_BASE_FILLER_H_
#define _U2_GT_RUNNABLES_SMITH_WATERMAN_DIALOG_BASE_FILLER_H_

#include "GTUtilsDialog.h"
#include "api/GTRegionSelector.h"

namespace U2 {

    class SmithWatermanDialogFiller : public Filler {
    public:
        enum Button {Search, Cancel};
        SmithWatermanDialogFiller(U2OpStatus &_os, const QString& _pattern = "", const GTRegionSelector::RegionSelectorSettings& _s = GTRegionSelector::RegionSelectorSettings()) : Filler(_os, "SmithWatermanDialogBase"), button(Search), pattern(_pattern), s(_s){}
        virtual void run();
        Button button;
    private:
        QString pattern;
        GTRegionSelector::RegionSelectorSettings s;
    };
}

#endif