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

class Filler : public Runnable {
public:
    Filler(U2OpStatus& _os, const QString& _objectName) : os(_os), objectName(_objectName){}
    QString getObjectName() const { return objectName; }
protected:
    U2OpStatus &os;
    QString objectName;
};

class GUIDialogWaiter : public QObject {
    Q_OBJECT
public:
    enum DialogType {Modal, Popup};
    GUIDialogWaiter(U2OpStatus &os, Runnable* _r, DialogType t, const QString& objectName, int timeout = 20000);
    virtual ~GUIDialogWaiter();

    static const int timerPeriod = 100;

    bool hadRun;
    int waiterId;
    QString objectName;

public slots:
    void checkDialog();

private:
    U2OpStatus &os;
    Runnable *runnable;
    DialogType type;

    QTimer* timer;
    int waitingTime;
    int timeout;

    void finishWaiting(); // deletes timer and runnable
    bool isExpectedName(const QString& widgetObjectName, const QString& expectedObjectName);
};

class GTUtilsDialog {
public:
    // if objectName is not empty, waits for QWidget with a given name
    static void waitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType = GUIDialogWaiter::Modal, const QString& objectName = "");

    // deletes all GUIDialogWaiters
    static void cleanup();
private:
    static QList<GUIDialogWaiter*> pool;
};

} // namespace

#endif
