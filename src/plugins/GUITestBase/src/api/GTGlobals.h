/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GT_GLOBALS_H_
#define _U2_GT_GLOBALS_H_

#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/Log.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QAction>
#else
#include <QtWidgets/QAction>
#endif

namespace U2 {

#define GT_DEBUG_MESSAGE(condition, errorMessage, result) \
{ \
    uiLog.trace("\n------------"); \
    uiLog.trace("GT_DEBUG_MESSAGE Checking condition <" #condition ">"); \
    uiLog.trace("GT_DEBUG_MESSAGE errorMessage <" + QString(errorMessage) + ">"); \
    if (condition) { \
        uiLog.trace("GT_DEBUG_MESSAGE ok"); \
    } \
    else { \
        uiLog.trace("GT_DEBUG_MESSAGE FAILED"); \
    } \
    if (os.hasError()) { \
        uiLog.trace("GT_DEBUG_MESSAGE OpStatus already has error"); \
        uiLog.trace("GT_DEBUG_MESSAGE OpStatus error <" + os.getError() + ">"); \
    } \
    uiLog.trace("------------\n"); \
}

#define CHECK_SET_ERR(condition, errorMessage) \
    CHECK_SET_ERR_RESULT(condition, errorMessage, )

#define CHECK_SET_ERR_RESULT(condition, errorMessage, result) \
{ \
    GT_DEBUG_MESSAGE(condition, errorMessage, result); \
    if (os.hasError()) { return result; } \
    CHECK_EXT(condition, if (!os.hasError()) { GTGlobals::GUITestFail(); os.setError(errorMessage);}, result) \
}

#define GT_CHECK(condition, errorMessage) \
    GT_CHECK_RESULT(condition, errorMessage, )

#define GT_CHECK_RESULT(condition, errorMessage, result) \
    CHECK_SET_ERR_RESULT(condition, GT_CLASS_NAME " __ " GT_METHOD_NAME " _  " + QString(errorMessage), result)

class GTGlobals {
public:
    enum UseMethod {UseMouse, UseKey, UseKeyBoard};
    enum WindowAction {Minimize, Maximize, Close, WindowActionCount};

    // if failIfNull is set to true, fails if object wasn't found
    class FindOptions {
    public:
        FindOptions(bool fail = true) : failIfNull(fail){}
        bool failIfNull;
    };

    static void sleep(int msec = 2000);
    static void sendEvent(QObject *obj, QEvent *e);
    static void takeScreenShot(QString path);
    static void GUITestFail(){uiLog.trace("\nGT_DEBUG_MESSAGE !!!FIRST FAIL");}
};

} //namespace

#endif
