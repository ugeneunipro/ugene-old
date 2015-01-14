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

#include "GTAction.h"
#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QAbstractButton>
#include <QtGui/QToolButton>
#else
#include <QtWidgets/QWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QToolButton>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTAction"

#define GT_METHOD_NAME "button"
QAbstractButton* GTAction::button(U2OpStatus &os, const QString &actionName, QObject *parent, const GTGlobals::FindOptions& options) {

    QAction* a = findAction(os, actionName, parent);
    if (!a) {
        a = findAction(os, actionName);
    }
    if (options.failIfNull) {
        GT_CHECK_RESULT(NULL != a, "Action is NULL!", NULL);
    } else if (NULL == a) {
            return NULL;
    }

    QList<QWidget*> associated = a->associatedWidgets();
    foreach(QWidget* w, associated) {
        QAbstractButton *tb = qobject_cast<QAbstractButton*>(w);
        if (tb) {
            if (parent) {
                QList<QToolButton*> childButtons = parent->findChildren<QToolButton*>(); // da. daa.
                if (childButtons.contains(dynamic_cast<QToolButton*>(tb))) {
                    return tb;
                }
            }
            else {
                return tb;
            }
        }
    }

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "button"
QAbstractButton* GTAction::button(U2OpStatus &os, const QAction* a, QObject *parent) {

    GT_CHECK_RESULT(a != NULL, "action is NULL", NULL);

    QList<QWidget*> associated = a->associatedWidgets();
    foreach(QWidget* w, associated) {
        QAbstractButton *tb = qobject_cast<QAbstractButton*>(w);
        if (tb) {
            if (parent) {
                QList<QToolButton*> childButtons = parent->findChildren<QToolButton*>(); // da. daa.
                if (childButtons.contains(dynamic_cast<QToolButton*>(tb))) {
                    return tb;
                }
            }
            else {
                return tb;
            }
        }
    }

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getAction"
QAction* GTAction::findAction(U2OpStatus &/*os*/, const QString &actionName, QObject *parent) {

    if (parent == NULL) {
        parent = AppContext::getMainWindow()->getQMainWindow();
    }
    QAction* a = parent->findChild<QAction*>(actionName);

    return a;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findActionByText"
QAction* GTAction::findActionByText(U2OpStatus &os, const QString &text, QObject *parent) {

    if (parent == NULL) {
        parent = AppContext::getMainWindow()->getQMainWindow();
    }
    QList<QAction*> list = parent->findChildren<QAction*>();
    QList<QAction*> resultList;

    foreach(QAction* act, list){
        if(act->text()==text){
            resultList<<act;
        }
    }

    GT_CHECK_RESULT(resultList.count()!=0,"action not found", NULL);
    GT_CHECK_RESULT(resultList.count()<2, QString("There are %1 actions with this text").arg(resultList.count()), NULL);

    return resultList.takeFirst();
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} //namespace
