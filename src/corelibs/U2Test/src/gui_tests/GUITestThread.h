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

#ifndef _U2_GUI_TEST_THREAD_H_
#define _U2_GUI_TEST_THREAD_H_

#include <QThread>
#include <U2Core/global.h>


namespace U2 {

class GUITest;
class Logger;
typedef QList<GUITest *> GUITests;

class U2TEST_EXPORT GUITestThread : public QThread {
    Q_OBJECT
public:
    GUITestThread(GUITest *test, Logger &log, bool needCleanup = true);

    void run();

    GUITest* getTest(){ return test; }
    QString getTestResult() { return testResult; }

private slots:
    void sl_testTimeOut();

private:
    QString launchTest(const GUITests &tests);

    static GUITests preChecks();
    static GUITests postChecks();
    static GUITests postActions();
    void clearSandbox();
    static void removeDir(const QString &dirName);
    void saveScreenshot();
    static void cleanup();
    void writeTestResult();

    GUITest *test;
    Logger &log;
    bool needCleanup;
    QString testResult;
};

}   // namespace U2

#endif // _U2_GUI_TEST_THREAD_H_
