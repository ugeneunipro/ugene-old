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

#ifndef _U2_GT_RUNNABLES_DOTPLOT_DIALOG_FILLER_H_
#define _U2_GT_RUNNABLES_DOTPLOT_DIALOG_FILLER_H_

#include "GTUtilsDialog.h"

namespace U2 {

    class DotPlotFiller : public Filler {
    public:
        DotPlotFiller(U2OpStatus &_os, int _minLen=100, int _identity = 0, bool _invertedRepeats = false, bool _but1kpressed=false) : Filler(_os, "DotPlotDialog"), minLen(_minLen), identity(_identity), invertedRepeats(_invertedRepeats), but1kpressed(_but1kpressed) {}
        virtual void run();
    private:
        int minLen, identity;
        bool invertedRepeats, but1kpressed;
    };


class DotPlotFilesDialogFiller : public Filler {
public:
class Parameters {
public:
Parameters():
line_firstFileEdit_text(""),
button_openFirstButton_clicked(false),
is_mergeFirstCheckBox_checked(0),
spin_gapFirst_value(0),
is_oneSequenceCheckBox_checked(0),
line_secondFileEdit_text(""),
button_openSecondButton_clicked(false),
is_mergeSecondCheckBox_checked(0),
spin_gapSecond_value(0),
button_startButton_clicked(false),
button_cancelButton_clicked(false){}


QString line_firstFileEdit_text;
bool button_openFirstButton_clicked;
bool is_mergeFirstCheckBox_checked;
int spin_gapFirst_value;
bool is_oneSequenceCheckBox_checked;
QString line_secondFileEdit_text;
bool button_openSecondButton_clicked;
bool is_mergeSecondCheckBox_checked;
int spin_gapSecond_value;
bool button_startButton_clicked;
bool button_cancelButton_clicked;
};


DotPlotFilesDialogFiller(U2OpStatus &os, Parameters* parameters) :
Filler(os, "DotPlotFilesDialog"),
parameters(parameters) {
CHECK_SET_ERR(parameters, "Invalid filler parameters: NULL pointer");
}

virtual void run();

private:

Parameters* parameters;

};

}


#endif
