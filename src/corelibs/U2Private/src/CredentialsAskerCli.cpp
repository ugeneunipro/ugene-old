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

#include <QtCore/qglobal.h>
#ifdef Q_OS_WIN
#include <conio.h>
#endif

#ifdef Q_OS_UNIX
#include <termios.h>
#endif

#include <iostream>
#include <stdio.h>

#include <U2Core/AppContext.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SafePoints.h>

#include "CredentialsAskerCli.h"

#ifdef Q_OS_UNIX // source: http://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-in-linux
static struct termios oldTerm, newTerm;

/* Initialize new terminal i/o settings */
void initTermios(int echo) {
    tcgetattr(0, &oldTerm); /* grab old terminal i/o settings */
    newTerm = oldTerm; /* make new settings same as old settings */
    newTerm.c_lflag &= ~ICANON; /* disable buffered i/o */
    newTerm.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &newTerm); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) {
    tcsetattr(0, TCSANOW, &oldTerm);
}

/* Read 1 character - echo defines echo mode */
char getchExt(int echo) {
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

/* Read 1 character without echo */
char _getch(void) {
    return getchExt(0);
}

#endif

static const QString BACKSPACE_PRINT_STR = "\b";
static const QString NEW_LINE_STR = "\n";
#ifdef Q_OS_WIN
static const QString BACKSPACE_STR = "\b";
static const QString RETURN_STR = "\r";
#else
static const QString BACKSPACE_STR = "\177";
static const QString RETURN_STR = "\n";
#endif

static const QString ASTERISC_STR = "*";
static const QString YES_STR = "Y";
static const QString NO_STR = "N";

static const QString ENTER_PWD_STR = QObject::tr("Enter password: ");
static const QString ENTER_LOGIN_STR = QObject::tr("Enter user name: ");

static const QString REMEMBER_PWD_QUESTION = QObject::tr("Would you like UGENE to remember the password?");
static const QString CONNECTION_START_STR = QObject::tr("Connect to the '%1' ...\n");
static const QString LOGIN_NOTIFICATION_STR = QObject::tr("You are going to log in as '%1'.\n");
static const QString USE_ANOTHER_LOGIN_QUESTION = QObject::tr("Would you like to log in as another user?");

namespace U2 {

namespace {

void printString(const QString &str) {
    std::cout << str.toLocal8Bit().constData();
}

QString getChar() {
    const char c = _getch();
    return QString::fromLocal8Bit(QByteArray(1, c));
}

QString getLine() {
    std::string result;
    getline(std::cin, result);
    return QString::fromStdString(result);
}

bool askYesNoQuestion(const QString &question) {
    QString readKey;
    int yes = -1;
    int no = -1;
    do {
        printString(question + QString(" (%1/%2)").arg(YES_STR).arg(NO_STR));
        readKey = getChar();
        yes = QString::compare(readKey, YES_STR, Qt::CaseInsensitive);
        no = QString::compare(readKey, NO_STR, Qt::CaseInsensitive);
        printString(NEW_LINE_STR);
    } while (0 != yes && 0 != no);

    return 0 == yes;
}

bool inputFinish(const QString &key) {
    return (NEW_LINE_STR == key) || (RETURN_STR == key) || (RETURN_STR + NEW_LINE_STR == key);
}

QString askPwd() {
    printString(ENTER_PWD_STR);

    QString pwd;

    QString readKey;
    do {
        readKey = getChar();

        if (readKey != BACKSPACE_STR && readKey != RETURN_STR) {
            pwd += readKey;
            printString(ASTERISC_STR);
        }
        else if (readKey == BACKSPACE_STR && !pwd.isEmpty()) {
            pwd.truncate(pwd.length() - 1);
            printString(BACKSPACE_PRINT_STR + " " + BACKSPACE_PRINT_STR);
        }
    } while (!inputFinish(readKey));

    printString(NEW_LINE_STR);
    return pwd;
}

}

bool CredentialsAskerCli::askWithFixedLogin(const QString &resourceUrl) const {
    SAFE_POINT(!AppContext::isGUIMode(), "Unexpected application run mode", false);

    QString userName;
    const QString shortDbiUrl = U2DbiUtils::full2shortDbiUrl(resourceUrl, userName);

    printString(CONNECTION_START_STR.arg(shortDbiUrl));
    printString(LOGIN_NOTIFICATION_STR.arg(userName));

    const QString pwd = askPwd();
    const bool isRemembered = askYesNoQuestion(REMEMBER_PWD_QUESTION);

    saveCredentials(resourceUrl, pwd, isRemembered);
    return true;
}

bool CredentialsAskerCli::askWithModifiableLogin(QString &resourceUrl) const {
    SAFE_POINT(!AppContext::isGUIMode(), "Unexpected application run mode", false);

    QString userName;
    const QString shortDbiUrl = U2DbiUtils::full2shortDbiUrl(resourceUrl, userName);

    printString(CONNECTION_START_STR.arg(shortDbiUrl));
    printString(LOGIN_NOTIFICATION_STR.arg(userName));
    const bool logAsAnotherUser = askYesNoQuestion(USE_ANOTHER_LOGIN_QUESTION);

    if (logAsAnotherUser) {
        do {
            printString(ENTER_LOGIN_STR);
            userName = getLine();
        } while (!userName.isEmpty());
        printString(NEW_LINE_STR);
    }

    const QString pwd = askPwd();
    const bool isRemembered = askYesNoQuestion(REMEMBER_PWD_QUESTION);

    resourceUrl = U2DbiUtils::createFullDbiUrl(userName, shortDbiUrl);
    saveCredentials(resourceUrl, pwd, isRemembered);
    return true;
}

} //namespace U2
