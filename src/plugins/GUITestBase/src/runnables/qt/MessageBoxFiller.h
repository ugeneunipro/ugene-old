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

#ifndef _U2_GT_RUNNABLES_MESSAGE_BOX_FILLER_H_
#define _U2_GT_RUNNABLES_MESSAGE_BOX_FILLER_H_

#include "GTUtilsDialog.h"

#include <QtGui/QMessageBox>

namespace U2 {

    class MessageBoxDialogFiller : public Filler {
    public:
        MessageBoxDialogFiller(U2OpStatus &_os, QMessageBox::StandardButton _b)
            :Filler(_os, ""), b(_b){}
        virtual void run();
    private:
        QMessageBox::StandardButton b;
    };

    class MessageBoxNoToAllOrNo : public Filler {
    public:
        MessageBoxNoToAllOrNo(U2OpStatus &_os) : Filler(_os, ""){}
        virtual void run();
    };

    class MessageBoxOpenAnotherProject : public Filler {
    public:
        MessageBoxOpenAnotherProject(U2OpStatus &_os) : Filler(_os, ""){}
        virtual void run();
    };
}

#endif