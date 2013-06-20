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

#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>
#include <QWebFrame>

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Task.h>

#include <U2Gui/MainWindow.h>

#include "OutputFilesWidget.h"
#include "ProblemsWidget.h"
#include "ResourcesWidget.h"
#include "StatisticsWidget.h"

#include "Dashboard.h"

namespace U2 {

/************************************************************************/
/* Dashboard */
/************************************************************************/
Dashboard::Dashboard(const WorkflowMonitor *monitor, QWidget *parent)
: QWebView(parent), _monitor(monitor), initialized(false)
{
    connect(this, SIGNAL(loadFinished(bool)), SLOT(sl_loaded(bool)));
    loadDocument();
}

void Dashboard::loadDocument() {
    QFile file(":U2Designer/html/Dashboard.html");
    bool opened = file.open(QIODevice::ReadOnly);
    SAFE_POINT(opened, "Can not load Dashboard.html", );
    QByteArray html = file.readAll();
    file.close();

    page()->mainFrame()->setHtml(html);
}

void Dashboard::sl_loaded(bool ok) {
    CHECK(!initialized, );
    SAFE_POINT(ok, "Loaded with errors", );
    initialized = true;
    page()->mainFrame()->addToJavaScriptWindowObject("agent", new JavascriptAgent());

    doc = page()->mainFrame()->documentElement();
    OutputFilesWidget *files = new OutputFilesWidget(addWidget(tr("Output Files"), 0), this);
    ResourcesWidget *resource = new ResourcesWidget(addWidget(tr("Workflow Task"), 1), this);
    StatisticsWidget *stat = new StatisticsWidget(addWidget(tr("Common Statistics"), 1), this);

    sl_runStateChanged(false);
    if (!monitor()->getProblems().isEmpty()) {
        sl_addProblemsWidget();
    }

    connect(monitor(), SIGNAL(si_runStateChanged(bool)), SLOT(sl_runStateChanged(bool)));
    connect(monitor(), SIGNAL(si_firstProblem()), SLOT(sl_addProblemsWidget()));
}

void Dashboard::sl_addProblemsWidget() {
    ProblemsWidget *problems = new ProblemsWidget(addWidget(tr("Problems")), this);
}

int Dashboard::containerSize(const QString &id) {
    QWebElement cont = doc.findFirst(id);
    SAFE_POINT(!cont.isNull(), "NULL container", 0);
    QWebElementCollection children = cont.findAll(".widget");
    return children.count();
}

QWebElement Dashboard::addWidget(const QString &title, int cntNum) {
    bool left = true;
    if (0 == cntNum) {
        left = true;
    } else if (1 == cntNum) {
        left = false;
    } else if (containerSize("#left-container") <= containerSize("#right-container")) {
        left = true;
    } else {
        left = false;
    }
    QWebElement container = doc.findFirst(left ? "#left-container" : "#right-container");
    container.appendInside(
        "<div class=\"widget\">"
            "<div class=\"title\"><div class=\"title-content\">" + title + "</div></div>"
            "<div class=\"widget-content\"></div>"
        "</div>");

    QWebElement widget = container.lastChild();
    return widget.findFirst(".widget-content");
}

const WorkflowMonitor * Dashboard::monitor() {
    return _monitor;
}

QWebElement Dashboard::getDocument() {
    return doc;
}

void Dashboard::sl_runStateChanged(bool paused) {
    QString script = paused ? "pauseTimer()" : "startTimer()";
    page()->mainFrame()->evaluateJavaScript(script);
}

void Dashboard::contextMenuEvent(QContextMenuEvent * /*ev*/) {

}

/************************************************************************/
/* DashboardWidget */
/************************************************************************/
DashboardWidget::DashboardWidget(const QWebElement &_container, Dashboard *parent)
: QObject(parent), dashboard(parent), container(_container)
{

}

/************************************************************************/
/* JavascriptAgent */
/************************************************************************/
void JavascriptAgent::openUrl(const QString &url) {
    Task *t = AppContext::getProjectLoader()->openWithProjectTask(url);
    if (t) {
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
}

void JavascriptAgent::openByOS(const QString &url) {
    if (!QFile::exists(url)) {
        QMessageBox::critical((QWidget*)AppContext::getMainWindow()->getQMainWindow(), tr("Error"), tr("The file does not exist"));
        return;
    }
    QDesktopServices::openUrl(QUrl("file:///" + url));
}

} // U2
