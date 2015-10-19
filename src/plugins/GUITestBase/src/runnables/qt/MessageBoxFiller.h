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

#ifndef _U2_GT_RUNNABLES_MESSAGE_BOX_FILLER_H_
#define _U2_GT_RUNNABLES_MESSAGE_BOX_FILLER_H_

#include <QMessageBox>

#include "GTUtilsDialog.h"

namespace U2 {

class MessageBoxDialogFiller : public Filler {
public:
    MessageBoxDialogFiller(U2OpStatus &os, QMessageBox::StandardButton b, const QString &message = "", const QString &objectName = "");
    MessageBoxDialogFiller(U2OpStatus &os, const QString &buttonText, const QString &message = "");

    virtual void commonScenario();

protected:
    QMessageBox::StandardButton b;
    QString buttonText;
    QString message;
};

class AppCloseMessageBoxDialogFiller : public Filler {
public:
    AppCloseMessageBoxDialogFiller(U2OpStatus &os);
    void commonScenario();
};

class MessageBoxNoToAllOrNo : public Filler {
public:
    MessageBoxNoToAllOrNo(U2OpStatus &os);
    void commonScenario();
};

class MessageBoxOpenAnotherProject : public Filler {
public:
    MessageBoxOpenAnotherProject(U2OpStatus &os);
    void commonScenario();
};

class InputIntFiller : public Filler {
public:
    InputIntFiller(U2OpStatus &os, int value);
    void commonScenario();
private:
    int value;
};

}   // namespace U2

#endif // _U2_GT_RUNNABLES_MESSAGE_BOX_FILLER_H_
