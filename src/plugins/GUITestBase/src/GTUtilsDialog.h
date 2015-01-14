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

#ifndef _U2_GUI_DIALOG_UTILS_H_
#define _U2_GUI_DIALOG_UTILS_H_

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QDialogButtonBox>
#endif

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
    enum DialogType {
        Modal,
        Popup
    };
    enum DialogDestiny {
        MustBeRun,
        MustNotBeRun,
        NoMatter
    };

    struct WaitSettings {
        WaitSettings(const QString& _objectName = "",
                     DialogType _dialogType = GUIDialogWaiter::Modal,
                     int _timeout = 120000,
                     DialogDestiny _destiny = MustBeRun) :
            dialogType(_dialogType),
            objectName(_objectName),
            timeout(_timeout),
            destiny(_destiny) {}

        DialogType dialogType;
        QString objectName;
        int timeout;
        DialogDestiny destiny;
    };

    GUIDialogWaiter(U2OpStatus &os, Runnable* _r, const WaitSettings& settings = WaitSettings());
    virtual ~GUIDialogWaiter();

    WaitSettings getSettings(){return settings;}
    void stopTimer();
    static const int timerPeriod = 100;

    bool hadRun;
    int waiterId;

public slots:
    void checkDialog();
    void checkDialogPool();

private:
    U2OpStatus &os;
    Runnable *runnable;
    WaitSettings settings;

    QTimer* timer;
    int waitingTime;

    void finishWaiting(); // deletes timer and runnable
    bool isExpectedName(const QString& widgetObjectName, const QString& expectedObjectName);
};

class CustomScenario {
public:
    virtual void run(U2OpStatus &os) = 0;
    virtual ~CustomScenario() {}
};

class Filler : public Runnable {
public:
    Filler(U2OpStatus &os, const GUIDialogWaiter::WaitSettings &settings, CustomScenario *scenario = NULL);
    Filler(U2OpStatus &os, const QString &objectName, CustomScenario *scenario = NULL);
    ~Filler();

    GUIDialogWaiter::WaitSettings getSettings() const;
    void run();
    virtual void commonScenario() {}

protected:
    U2OpStatus &os;
    GUIDialogWaiter::WaitSettings settings;
    CustomScenario *scenario;
};

class GTUtilsDialog{
    friend class TimerLauncher;
    friend class GUIDialogWaiter;
public:
    enum CleanupSettings {
        FailOnUnfinished, NoFailOnUnfinished
    };

    static QDialogButtonBox * buttonBox(U2OpStatus &os, QWidget *dialog);

    static void clickButtonBox(U2OpStatus &os, QDialogButtonBox::StandardButton button);
    static void clickButtonBox(U2OpStatus &os, QWidget *dialog, QDialogButtonBox::StandardButton button);

    // if objectName is not empty, waits for QWidget with a given name
    static void waitForDialog(U2OpStatus &os, Runnable *r, const GUIDialogWaiter::WaitSettings& settings);

    static void waitForDialog(U2OpStatus &os, Runnable *r);

    static void waitForDialogWhichMustNotBeRunned(U2OpStatus &os, Runnable *r);

    static void waitForDialogWhichMayRunOrNot(U2OpStatus &os, Runnable *r);

    // deletes all GUIDialogWaiters, sets err if there are unfinished waiters
    static void cleanup(U2OpStatus &os, CleanupSettings s = FailOnUnfinished);
private:
    static void checkAllFinished(U2OpStatus &os);

    static QList<GUIDialogWaiter*> pool;
    static const int timerPeriod = 100;
};

} // namespace

#endif
