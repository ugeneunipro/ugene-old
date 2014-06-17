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

#include "api/GTGlobals.h"
#include <U2Core/AppContext.h>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtTest/QSpontaneKeyEvent>
#include <QtTest>
#include <QtGui/QPixmap>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#endif

namespace U2 {

void GTGlobals::sleep(int msec) {
    QTest::qWait((msec));
}

void GTGlobals::sendEvent(QObject *obj, QEvent *e) {
    QSpontaneKeyEvent::setSpontaneous(e);
    qApp->notify(obj, e);
}

void GTGlobals::takeScreenShot(QString path){

    QPixmap originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
    originalPixmap.save(path);
}

GTGlobals::FindOptions::FindOptions(bool fail) :
    depth(INFINITE_DEPTH),
    failIfNull(fail)
{
}

void GTGlobals::GUITestFail(){
    uiLog.trace("\nGT_DEBUG_MESSAGE !!!FIRST FAIL");
}

} //namespace

