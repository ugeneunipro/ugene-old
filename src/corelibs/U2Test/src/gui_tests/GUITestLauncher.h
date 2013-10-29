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

#ifndef _U2_GUI_TEST_LAUNCHER_H_
#define _U2_GUI_TEST_LAUNCHER_H_

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MultiTask.h>
#include <U2Gui/MainWindow.h>
#include <QtCore/QProcessEnvironment>

namespace U2 {

class GUITest;

class GUITestLauncher: public Task {
    Q_OBJECT
public:
    GUITestLauncher(int _suiteNumber=0);

    virtual void run();
    virtual QString generateReport() const;

private:
    QList<GUITest *> tests;
    QMap<QString, QString> results;
    int suiteNumber;

    static QStringList getTestProcessArguments(const QString &testName);
    static QProcessEnvironment getProcessEnvironment(const QString &testName);
    static QString testOutFile(const QString &testName);

    void firstTestRunCheck(const QString& testName);
    static QString performTest(const QString& testName, bool isLong);
    static QString readTestResult(const QByteArray& output);
    static bool renameTestLog(const QString& testName);

    bool initGUITestBase();
    void updateProgress(int finishedCount);
};


} // namespace

#endif
