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
 
#ifndef _U2_GUI_MAFFT_SUPPORT_RUN_DIALOG_FILLER_H_
#define _U2_GUI_MAFFT_SUPPORT_RUN_DIALOG_FILLER_H_
 
#include "utils/GTUtilsDialog.h"
#include <base_dialogs/GTFileDialog.h>

namespace U2 {
using namespace HI;

class MAFFTSupportRunDialogFiller : public Filler {
public:
    class Parameters {
        public:
        Parameters():
            ckeckBox_gapOpenCheckBox_checked(0),
            doubleSpin_gapOpenSpinBox_value(1.53),
            ckeckBox_gapExtCheckBox_checked(0),
            doubleSpin_gapExtSpinBox_value(0),
            ckeckBox_maxNumberIterRefinementCheckBox_checked(0),
            spin_maxNumberIterRefinementSpinBox_value(0){}

        bool ckeckBox_gapOpenCheckBox_checked;
        double doubleSpin_gapOpenSpinBox_value;
        bool ckeckBox_gapExtCheckBox_checked;
        double doubleSpin_gapExtSpinBox_value;
        bool ckeckBox_maxNumberIterRefinementCheckBox_checked;
        int spin_maxNumberIterRefinementSpinBox_value;
    };


    MAFFTSupportRunDialogFiller(HI::GUITestOpStatus &os, Parameters* parameters) :
        Filler(os, "MAFFTSupportRunDialog"),
        parameters(parameters) {
            CHECK_SET_ERR(parameters, "Invalid filler parameters: NULL pointer");
    }

void commonScenario();

private:

Parameters* parameters;

};

}

#endif

 
