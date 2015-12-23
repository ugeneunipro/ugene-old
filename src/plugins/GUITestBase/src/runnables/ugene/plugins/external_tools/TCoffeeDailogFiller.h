/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_TCOFFEE_DAILOG_FILLER_H_
#define _U2_TCOFFEE_DAILOG_FILLER_H_

#include "utils/GTUtilsDialog.h"
#include <limits.h>

namespace U2{
using namespace HI;

class TCoffeeDailogFiller : public Filler {
public:
    TCoffeeDailogFiller(HI::GUITestOpStatus& os, int gapOpen = INT_MAX, int gapExt = INT_MAX, int numOfIters = INT_MAX);

    void commonScenario();

private:
    int gapOpen;
    int gapExt;
    int numOfIters;
};

}
#endif // _U2_TCOFFEE_DAILOG_FILLER_H_
