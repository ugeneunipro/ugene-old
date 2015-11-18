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

#ifndef _HI_GUI_DIALOG_UTILS_H_
#define _HI_GUI_DIALOG_UTILS_H_

#include <QDialogButtonBox>

#include <U2Test/CustomScenario.h>

#include "GTGlobals.h"

namespace HI {

class HI_EXPORT Runnable {
public:
    virtual void run() = 0;
    virtual ~Runnable(){}
};

class HI_EXPORT GUIDialogWaiter : public QObject {
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

    GUIDialogWaiter(U2::U2OpStatus &os, Runnable* _r, const WaitSettings& settings = WaitSettings());
    virtual ~GUIDialogWaiter();

    WaitSettings getSettings(){return settings;}
    void stopTimer();
    static const int timerPeriod = 100;

    bool hadRun;
    int waiterId;

    bool isExpectedName(const QString& widgetObjectName, const QString& expectedObjectName);
public slots:
    void checkDialog();
    void checkDialogPool();

private:
    U2::U2OpStatus &os;
    Runnable *runnable;
    WaitSettings settings;

    QTimer* timer;
    int waitingTime;

    void finishWaiting(); // deletes timer and runnable

};

class HI_EXPORT Filler : public Runnable {
public:
    Filler(U2::U2OpStatus &os, const GUIDialogWaiter::WaitSettings &settings, U2::CustomScenario *scenario = NULL);
    Filler(U2::U2OpStatus &os, const QString &objectName, U2::CustomScenario *scenario = NULL);
    ~Filler();

    GUIDialogWaiter::WaitSettings getSettings() const;
    void run();
    virtual void commonScenario() {}

protected:
    U2::U2OpStatus &os;
    GUIDialogWaiter::WaitSettings settings;
    U2::CustomScenario *scenario;
};

class HI_EXPORT HangChecker: public QObject{
    Q_OBJECT
public:
    HangChecker(U2::U2OpStatus &_os);
    QTimer* timer;
    void startChecking();
    U2::U2OpStatus &os;
    bool mightHung;
public slots:
    void sl_check();
};

class HI_EXPORT GTUtilsDialog{
    friend class TimerLauncher;
    friend class GUIDialogWaiter;
    friend class HangChecker;
public:
    enum CleanupSettings {
        FailOnUnfinished, NoFailOnUnfinished
    };

    static QDialogButtonBox * buttonBox(U2::U2OpStatus &os, QWidget *dialog);

    static void clickButtonBox(U2::U2OpStatus &os, QDialogButtonBox::StandardButton button);
    static void clickButtonBox(U2::U2OpStatus &os, QWidget *dialog, QDialogButtonBox::StandardButton button);

    // if objectName is not empty, waits for QWidget with a given name
    static void waitForDialog(U2::U2OpStatus &os, Runnable *r, const GUIDialogWaiter::WaitSettings& settings);

    static void waitForDialog(U2::U2OpStatus &os, Runnable *r, int timeout = 0);

    static void waitForDialogWhichMustNotBeRunned(U2::U2OpStatus &os, Runnable *r);

    static void waitForDialogWhichMayRunOrNot(U2::U2OpStatus &os, Runnable *r);

    // deletes all GUIDialogWaiters, sets err if there are unfinished waiters
    static void cleanup(U2::U2OpStatus &os, CleanupSettings s = FailOnUnfinished);

    static void startHangChecking(U2::U2OpStatus &os);
    static void stopHangChecking();

    static bool isButtonEnabled(U2::U2OpStatus& os, QWidget* dialog, QDialogButtonBox::StandardButton button);

private:
    static void checkAllFinished(U2::U2OpStatus &os);

    static QList<GUIDialogWaiter*> pool;
    static HangChecker* hangChecker;
    static const int timerPeriod = 100;
};

} // namespace

#endif
