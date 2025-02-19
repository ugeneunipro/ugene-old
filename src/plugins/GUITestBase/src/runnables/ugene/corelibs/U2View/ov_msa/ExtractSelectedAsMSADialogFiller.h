/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GT_RUNNABLES_EXTRACT_SELECTED_AS_MSA_DIALOG_FILLER_H_
#define _U2_GT_RUNNABLES_EXTRACT_SELECTED_AS_MSA_DIALOG_FILLER_H_

#include "utils/GTUtilsDialog.h"

namespace U2 {
using namespace HI;

class ExtractSelectedAsMSADialogFiller : public Filler {
public:
    ExtractSelectedAsMSADialogFiller(HI::GUITestOpStatus &os,QString _filepath, QStringList _list, int _from=0, int _to=0,bool _addToProj=true,
                                     bool _invertButtonPress=false,bool _allButtonPress=false,
                                     bool _noneButtonPress=false, bool _dontCheckFilepath = false, QString format = "") : Filler(os, "CreateSubalignmentDialog"),
        filepath(_filepath),
        format(format),
        list(_list),
        from(_from),
        to(_to),
        addToProj(_addToProj),
        invertButtonPress(_invertButtonPress),
        allButtonPress(_allButtonPress),
        noneButtonPress(_noneButtonPress),
        dontCheckFilepath(_dontCheckFilepath) {}
    ExtractSelectedAsMSADialogFiller(HI::GUITestOpStatus &os, CustomScenario *c): Filler(os, "CreateSubalignmentDialog", c){}
    virtual void commonScenario();
    static QStringList getSequences(HI::GUITestOpStatus &os, bool selected = true);
private:
    QString filepath;
    QString format;
    QStringList list;
    int from,to;
    bool addToProj, invertButtonPress, allButtonPress, noneButtonPress, dontCheckFilepath;
};

}

#endif
