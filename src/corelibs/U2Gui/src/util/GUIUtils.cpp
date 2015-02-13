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

#include "GUIUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/L10n.h>
#include <U2Core/Settings.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/AppSettingsGUI.h>
#include <U2Gui/MainWindow.h>

#include <QtCore/QFile>
#include <QtCore/QProcess>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#endif
#include <QtGui/QPainter>

#include <QUrl>
#include <QDesktopServices>

namespace U2 {

QAction* GUIUtils::getCheckedAction(QList<QAction*> actions){
        foreach(QAction* action, actions) {
            if(action->isChecked()){
                return action;
            }
        }
        return NULL;
}

QAction* GUIUtils::findActionByData(QList<QAction*> actions, const QString& data){
        foreach(QAction* action, actions) {
            if(action->data() == data){
                return action;
            }
        }
        return NULL;
}

QAction *GUIUtils::findActionByTooltip(QList<QAction *> actions, const QString &tooltip) {
    foreach (QAction* action, actions) {
        if (action->toolTip() == tooltip) {
            return action;
        }
    }
    return NULL;
}


QAction* GUIUtils::findAction(const QList<QAction*>& actions, const QString& name) {
    foreach(QAction* a, actions) {
        const QString& aname =  a->objectName();
        if (aname == name) {
            return a;
        }
    }
    return NULL;
}

QAction* GUIUtils::findActionAfter(const QList<QAction*>& actions, const QString& name) {
    bool found = false;
    foreach(QAction* a, actions) {
        if (found) {
            return a;
        }
        const QString& aname =  a->objectName();
        if (aname == name) {
            found = true;
        }
    }
    if (found) {
        return NULL;
    }
    return actions.first();
}


QMenu* GUIUtils::findSubMenu(QMenu* m, const QString& name) {
    QAction* action = findAction(m->actions(), name);
    if (action == NULL) {
        return NULL;
    }
    return action->menu();
}

void GUIUtils::disableEmptySubmenus(QMenu* m) {
    foreach(QAction* action, m->actions()) {
        QMenu* am = action->menu();
        if (am!=NULL && am->actions().isEmpty()) {
            action->setEnabled(false);
        }
    }
}

QIcon  GUIUtils::createSquareIcon(const QColor& c, int size) {
    int w = size;
    int h = size;
    QPixmap pix(w, h);
    QPainter p(&pix);
    p.setPen(Qt::black);
    p.drawRect(0, 0, w-1, h-1);
    p.fillRect(1, 1, w-2, h-2, c);
    p.end();
    return QIcon(pix);
}

QIcon  GUIUtils::createRoundIcon(const QColor& c, int size) {
    int w = size;
    int h = size;

    QPixmap pix(w, h);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    QPainterPath ep;
    //ep.addEllipse(1, 1, w-2, h-2);
    ep.addEllipse(0, 0, w-1, h-1);
    p.fillPath(ep, c);
    p.setPen(Qt::black);
    p.drawEllipse(0, 0, w-1, h-1);
    p.end();
    return QIcon(pix);
}

bool GUIUtils::runWebBrowser(const QString& url) {
    bool useDefaultWebBrowser = AppContext::getAppSettings()->getUserAppsSettings()->useDefaultWebBrowser();
    if (url.isEmpty()) {
        QMessageBox::critical(NULL, tr("Error!"), tr("Document URL is empty!"));
        return false;
    }
    
    QString program = AppContext::getAppSettings()->getUserAppsSettings()->getWebBrowserURL();
    bool ok = !program.isEmpty() && QFile(program).exists();

    if(useDefaultWebBrowser){
        bool launched = QDesktopServices::openUrl(QUrl(url));
        if (!launched){
            QMessageBox::critical(NULL, tr("Error!"), tr("Unable to launch default web browser."));
            AppContext::getAppSettings()->getUserAppsSettings()->setUseDefaultWebBrowser(false);
            AppContext::getAppSettingsGUI()->showSettingsDialog(APP_SETTINGS_USER_APPS);            
            program = AppContext::getAppSettings()->getUserAppsSettings()->getWebBrowserURL();
            ok = !program.isEmpty() && QFile(program).exists();
            if (!ok) {
                return false;
            }
            QString p = program;
            QStringList arguments;
            arguments.append(url);
            QProcess myProcess;
            return myProcess.startDetached(program, arguments);
        }else {
            return launched;
        }
    }else{
        if (!ok) {
            QMessageBox::critical(NULL, tr("Error!"), tr("Please specify the browser executable"));
            AppContext::getAppSettingsGUI()->showSettingsDialog(APP_SETTINGS_USER_APPS);
            program = AppContext::getAppSettings()->getUserAppsSettings()->getWebBrowserURL();

        ok = !program.isEmpty() && QFile(program).exists();
        }
        if (!ok) {
            return false;
        }
        QString p = program;
        QStringList arguments;
        arguments.append(url);
        QProcess myProcess;
        return myProcess.startDetached(program, arguments);
    }
}


bool GUIUtils::isMutedLnF(QTreeWidgetItem* item) {
    static QBrush disabledBrush;
    if (disabledBrush.style() == Qt::NoBrush) {
        disabledBrush = QApplication::palette().brush(QPalette::Disabled, QPalette::Foreground);
    }
    return item->foreground(0) == disabledBrush;
}

void GUIUtils::setMutedLnF( QTreeWidgetItem* item, bool enableMute, bool recursive) {
    QPalette::ColorGroup colorGroup = enableMute ? QPalette::Disabled : QPalette::Active;
    QBrush brush = QApplication::palette().brush(colorGroup, QPalette::Foreground);
    int count = item->columnCount();
    for (int column = 0; column < count; column++) {
        item->setForeground(column, brush);
    }
    if (recursive) {
        int childCount = item->childCount();
        for (int i = 0; i < childCount; ++i) {
            QTreeWidgetItem* childItem = item->child(i);
            if (!isMutedLnF(childItem)) {
                setMutedLnF(childItem, enableMute, recursive);
            }
        }
    }
}

static const QColor warningColor(255,200,200);
static const QColor okColor(255,255,255);

void GUIUtils::setWidgetWarning(QWidget *widget, bool value) {
    QColor color = value ? warningColor : okColor;
    QPalette p = widget->palette();
    p.setColor(QPalette::Active, QPalette::Base, color);
    widget->setPalette(p);
}

} //endif
