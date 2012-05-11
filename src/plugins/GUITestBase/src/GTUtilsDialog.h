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

#ifndef _U2_GUI_DIALOG_UTILS_H_
#define _U2_GUI_DIALOG_UTILS_H_

#include "api/GTGlobals.h"

namespace U2 {

class Runnable {
public:
    virtual void run() = 0;
    virtual ~Runnable(){}
};

class GUIDialogWaiter : public QObject {
    Q_OBJECT
public:
    enum DialogType {Modal, Popup};
    GUIDialogWaiter(Runnable* _r, DialogType t) : hadRun(false), r(_r), type(t){}
    bool hadRun;
public slots:
    void wait();
private:
    Runnable *r;
    DialogType type;
};

class GTUtilsDialog {
public:
    static void waitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType = GUIDialogWaiter::Modal, bool failOnNoDialog = true);
    static void preWaitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType = GUIDialogWaiter::Modal, int msec = 100);
};

} // namespace

#endif
