/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QWebElement>
#include <QWebFrame>

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/U2SafePoints.h>

#include "WelcomePageController.h"
#include "main_window/MainWindowImpl.h"

#include "WelcomePageWidget.h"

namespace U2 {

namespace {
    const int MAX_RECENT = 7;
}

WelcomePageWidget::WelcomePageWidget(QWidget *parent, WelcomePageController *controller)
    : MultilingualHtmlView(":ugene/html/welcome_page.html", parent),
      controller(controller)
{
    installEventFilter(this);
    setObjectName("webView");
    addController();
}

void WelcomePageWidget::sl_loaded(bool ok) {
    MultilingualHtmlView::sl_loaded(ok);
    addController();
}

void WelcomePageWidget::updateRecent(const QStringList &recentProjects, const QStringList &recentFiles) {
    updateRecentFilesContainer("recent_projects", recentProjects, tr("No opened projects yet"));
    updateRecentFilesContainer("recent_files", recentFiles, tr("No opened files yet"));
    page()->mainFrame()->evaluateJavaScript("updateLinksVisibility()");
}

void WelcomePageWidget::updateRecentFilesContainer(const QString &id, const QStringList &files, const QString &message) {
    static const QString divTemplate = "<div id=\"%1\" class=\"recent_items_content\">%2</div>";
    static const QString linkTemplate = "<a class=\"recentLink\" href=\"#\" onclick=\"ugene.openFile('%1')\" title=\"%1\">- %2</a>";

    QWebElement doc = page()->mainFrame()->documentElement();
    QWebElement recentFilesDiv = doc.findFirst("#" + id);
    SAFE_POINT(!recentFilesDiv.isNull(), "No recent files container", );
    recentFilesDiv.removeAllChildren();

    QStringList links;
    foreach (const QString &file, files.mid(0, MAX_RECENT)) {
        if (file.isEmpty()) {
            continue;
        }
        links << linkTemplate.arg(file).arg(QFileInfo(file).fileName());
    }
    QString result = message;
    if (!links.isEmpty()) {
        result = links.join("\n");
    }
    recentFilesDiv.setOuterXml(divTemplate.arg(id).arg(result));
}

void WelcomePageWidget::addController() {
    page()->mainFrame()->addToJavaScriptWindowObject("ugene", controller);
    controller->onPageLoaded();
}

void WelcomePageWidget::dragEnterEvent(QDragEnterEvent *event) {
    MainWindowDragNDrop::dragEnterEvent(event);
}

void WelcomePageWidget::dropEvent(QDropEvent *event) {
    MainWindowDragNDrop::dropEvent(event);
}

void WelcomePageWidget::dragMoveEvent(QDragMoveEvent *event) {
    MainWindowDragNDrop::dragMoveEvent(event);
}

bool WelcomePageWidget::eventFilter(QObject *watched, QEvent *event) {
    CHECK(this == watched, false);
    switch (event->type()) {
        case QEvent::DragEnter:
            dragEnterEvent(dynamic_cast<QDragEnterEvent*>(event));
            return true;
        case QEvent::DragMove:
            dragMoveEvent(dynamic_cast<QDragMoveEvent*>(event));
            return true;
        case QEvent::Drop:
            dropEvent(dynamic_cast<QDropEvent*>(event));
            return true;
        case QEvent::FocusIn:
            setFocus();
            return true;
        default:
            return false;
    }
}

} // U2
