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
#include <QtGui/QMessageBox>
#include <QtCore/QTimer>
#include "GTUtilsDialog.h"
#include <QDir>

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
    class CreateAnnotationDialogChecker : public Runnable {
    public:
        CreateAnnotationDialogChecker(U2OpStatus &_os) : os(_os){}
        virtual void run();
    private:
        U2OpStatus &os;
    };

    class ExportProjectDialogFiller : public Runnable {
    public:
        ExportProjectDialogFiller(U2OpStatus &_os, const QString &_projectFolder, const QString &_projectName)
            :os(_os), projectFolder(_projectFolder), projectName(_projectName){}
        virtual void run();
    private:
        U2OpStatus &os;
        const QString &projectFolder;
        const QString &projectName;
    };

    class ExportProjectDialogChecker : public Runnable {
    public:
        ExportProjectDialogChecker(U2OpStatus &_os, const QString &_projectName)
            :os(_os), projectName(_projectName){}
        virtual void run();
    private:
        U2OpStatus &os;
        const QString &projectName;
    };

    class SaveProjectAsDialogFiller : public Runnable {
    public:
        SaveProjectAsDialogFiller(U2OpStatus &_os, const QString &_projectName, const QString &_projectFolder, const QString &_projectFile)
            :os(_os), projectName(_projectName), projectFolder(_projectFolder), projectFile(_projectFile){}
        virtual void run();
    private:
        U2OpStatus &os;
        const QString &projectName;
        const QString &projectFolder;
        const QString &projectFile;
    };

    class MessageBoxDialogFiller : public Runnable {
    public:
        MessageBoxDialogFiller(U2OpStatus &_os, QMessageBox::StandardButton _b)
            :os(_os), b(_b){}
        virtual void run();
    private:
        U2OpStatus &os;
        QMessageBox::StandardButton b;
    };

    class PopupChooser : public Runnable {
    public:
        PopupChooser(U2OpStatus &_os, const QStringList &_namePath, GTGlobals::UseMethod _useMethod = GTGlobals::UseMouse)
            :os(_os), namePath(_namePath), useMethod(_useMethod){}
        virtual void run();
    private:
        U2OpStatus &os;
        QStringList namePath;
        GTGlobals::UseMethod useMethod;
    };

    class RemoteDBDialogFiller : public Runnable {
    public:
        RemoteDBDialogFiller(U2OpStatus &_os, const QString _resID, int _DBItemNum, bool _pressCancel = false, const QString _saveDirPath = QString(),  GTGlobals::UseMethod _useMethod = GTGlobals::UseMouse)
            :os(_os), resID(_resID), DBItemNum(_DBItemNum), pressCancel(_pressCancel), saveDirPath(_saveDirPath), useMethod(_useMethod){}
        virtual void run();
    private:
        U2OpStatus &os;
        QString resID;
        int DBItemNum;
        bool pressCancel;
        QString saveDirPath;
        GTGlobals::UseMethod useMethod;
    };

    class ExportToSequenceFormatFiller : public Runnable {
    public:
        ExportToSequenceFormatFiller(U2OpStatus &_os, const QString &_path, const QString &_name, GTGlobals::UseMethod method):
            os(_os), name(_name), useMethod(method) {
            QString __path = QDir::cleanPath(QDir::currentPath() + "/" + _path);
            if (__path.at(__path.count() - 1) != '/') {
                __path += '/';
            }

            path = __path;
        }
        virtual void run();
    private:
        U2OpStatus &os;
        QString path, name;
        GTGlobals::UseMethod useMethod;
    };

    static void waitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType = GUIDialogWaiter::Modal, bool failOnNoDialog = true);
    static void preWaitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType = GUIDialogWaiter::Modal);
};

} // namespace

#endif
