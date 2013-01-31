/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "GTUtilsDialog.h"

namespace U2 {

    class EditQualifierFiller : public Filler {
    public:
        EditQualifierFiller(U2OpStatus &_os, const QString &_qualifierName, const QString &_valueName, GTGlobals::UseMethod method = GTGlobals::UseMouse)
            :Filler(_os, "EditQualifierDialog"), qualifierName(_qualifierName), valueName(_valueName), useMethod(method){}
        virtual void run();
    private:
        QString qualifierName;
        QString valueName;
        GTGlobals::UseMethod useMethod;
    };

    class RenameQualifierFiller : public Filler {
    public:
        RenameQualifierFiller(U2OpStatus &_os, const QString &_newName, GTGlobals::UseMethod method = GTGlobals::UseMouse)
            :Filler(_os, ""), newName(_newName), useMethod(method){}
        virtual void run();
    private:
        QString newName;
        GTGlobals::UseMethod useMethod;
        };
}

#endif