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

    struct WaitSettings {
        WaitSettings(const QString& _objectName="", DialogType _dialogType = GUIDialogWaiter::Modal, int _timeout = 120000) : dialogType(_dialogType), objectName(_objectName), timeout(_timeout){}

        DialogType dialogType;
        QString objectName;
        int timeout;
    };

    GUIDialogWaiter(U2OpStatus &os, Runnable* _r, const WaitSettings& settings = WaitSettings());
    virtual ~GUIDialogWaiter();

    static const int timerPeriod = 100;

    bool hadRun;
    int waiterId;

public slots:
    void checkDialog();

private:
    U2OpStatus &os;
    Runnable *runnable;
    WaitSettings settings;

    QTimer* timer;
    int waitingTime;

    void finishWaiting(); // deletes timer and runnable
    bool isExpectedName(const QString& widgetObjectName, const QString& expectedObjectName);
};

class Filler : public Runnable {
public:
    Filler(U2OpStatus& _os, const GUIDialogWaiter::WaitSettings& _settings) : os(_os), settings(_settings){}
    Filler(U2OpStatus& _os, const QString &_objectName) : os(_os), settings(GUIDialogWaiter::WaitSettings(_objectName)){}

    GUIDialogWaiter::WaitSettings getSettings() const { return settings; }
protected:
    U2OpStatus &os;
    GUIDialogWaiter::WaitSettings settings;
};

class GTUtilsDialog {
public:
    enum CleanupSettings {
        FailOnUnfinished, NoFailOnUnfinished
    };

    // if objectName is not empty, waits for QWidget with a given name
    static void waitForDialog(U2OpStatus &os, Runnable *r, const GUIDialogWaiter::WaitSettings& settings);

    static void waitForDialog(U2OpStatus &os, Runnable *r);

    // deletes all GUIDialogWaiters, sets err if there are unfinished waiters
    static void cleanup(U2OpStatus &os, CleanupSettings s = FailOnUnfinished);
private:
    static void checkAllFinished(U2OpStatus &os);

    static QList<GUIDialogWaiter*> pool;
};

} // namespace

#endif
