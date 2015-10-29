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

#ifndef _U2_GT_RUNNABLES_EDIT_QUALIFIER_DIALOG_FILLER_H_
#define _U2_GT_RUNNABLES_EDIT_QUALIFIER_DIALOG_FILLER_H_

#include "utils/GTUtilsDialog.h"

namespace U2 {
using namespace HI;
class EditQualifierFiller : public Filler {
public:
    EditQualifierFiller(U2OpStatus &_os, const QString &_qualifierName, const QString &_valueName, bool _noCheck = false, bool _closeErrorMessageBox = false)
        :Filler(_os, "EditQualifierDialog"), qualifierName(_qualifierName), valueName(_valueName), noCheck(_noCheck), closeErrormessageBox(_closeErrorMessageBox){}
    virtual void run();
private:
    QString qualifierName;
    QString valueName;
    bool noCheck;
    bool closeErrormessageBox;
};

class RenameQualifierFiller : public Filler {
public:
    RenameQualifierFiller(U2OpStatus &_os, const QString &_newName)
        :Filler(_os, ""), newName(_newName) {}
    virtual void run();
private:
    QString newName;
};

}

#endif
