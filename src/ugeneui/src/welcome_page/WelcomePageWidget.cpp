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

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QWebElement>
#include <QWebFrame>

#include <U2Core/U2SafePoints.h>

#include "WelcomePageController.h"

#include "WelcomePageWidget.h"

namespace U2 {

namespace {
    const int MAX_RECENT = 7;
}

WelcomePageWidget::WelcomePageWidget(QWidget *parent, WelcomePageController *controller)
: QWidget(parent), loaded(false), controller(controller)
{
    setupUi(this);
    loadPage();
}

bool WelcomePageWidget::isLoaded() const {
    return loaded;
}

void WelcomePageWidget::sl_loaded(bool ok) {
    SAFE_POINT(ok, "Can not load page", );
    loaded = true;
    webView->page()->mainFrame()->addToJavaScriptWindowObject("ugene", controller);
    controller->onPageLoaded();
}

void WelcomePageWidget::loadPage() {
    QFile file(":ugene/html/welcome_page.html");
    bool opened = file.open(QIODevice::ReadOnly);
    SAFE_POINT(opened, "Can not load Welcome Page", );

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString html = stream.readAll();
    file.close();

    connect(webView, SIGNAL(loadFinished(bool)), SLOT(sl_loaded(bool)));
    webView->page()->mainFrame()->setHtml(html);
}

void WelcomePageWidget::updateRecent(const QStringList &recentProjects, const QStringList &recentFiles) {
    updateRecentFilesContainer("recent_projects", recentProjects, tr("No opened projects yet"));
    updateRecentFilesContainer("recent_files", recentFiles, tr("No opened files yet"));
}

void WelcomePageWidget::updateRecentFilesContainer(const QString &id, const QStringList &files, const QString &message) {
    static const QString divTemplate = "<div id=\"%1\" class=\"recent_items_content\">%2</div>";
    static const QString linkTemplate = "<a href=\"#\" onclick=\"ugene.openFile('%1')\">- %2</a>";

    QWebElement doc = webView->page()->mainFrame()->documentElement();
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
        result = links.join("<br>");
    }
    recentFilesDiv.setOuterXml(divTemplate.arg(id).arg(result));
}

} // U2
