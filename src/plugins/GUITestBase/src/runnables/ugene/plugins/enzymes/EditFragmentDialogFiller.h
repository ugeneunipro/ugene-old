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

#ifndef _U2_EDIT_FRAGMENT_DIALOG_FILLER_H_
#define _U2_EDIT_FRAGMENT_DIALOG_FILLER_H_

#include "utils/GTUtilsDialog.h"
#include <base_dialogs/GTFileDialog.h>

namespace U2 {
using namespace HI;

class EditFragmentDialogFiller : public Filler {
public:
    class Parameters {
    public:
        Parameters():
              lSticky(false),
              lCustom(false),
              lDirect(false),
              rSticky(false),
              rCustom(false),
              rDirect(false),
              checkRComplText(false)
        {
        }

        bool lSticky;
        bool lCustom;
        bool lDirect;
        QString lDirectText;
        QString lComplText;

        bool rSticky;
        bool rCustom;
        bool rDirect;
        QString rDirectText;
        QString rComplText;

        // GUITest_regression_scenarios_test_0574
        bool checkRComplText;
    };

    EditFragmentDialogFiller(HI::GUITestOpStatus &os, const Parameters &parameters);

    void commonScenario();

private:
    Parameters parameters;
};

} // U2

#endif // _U2_EDIT_FRAGMENT_DIALOG_FILLER_H_
