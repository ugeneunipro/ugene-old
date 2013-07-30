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
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Task.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>

#include <U2Lang/WorkflowSettings.h>

#include "OutputFilesWidget.h"
#include "ParametersWidget.h"
#include "ProblemsWidget.h"
#include "ResourcesWidget.h"
#include "StatisticsWidget.h"

#include "Dashboard.h"

namespace U2 {

static const QString REPORT_SUB_DIR("report/");
static const QString DB_FILE_NAME("dashboard.html");
static const QString SETTINGS_FILE_NAME("settings.ini");
static const QString OPENED_SETTING("opened");
static const QString NAME_SETTING("name");

/************************************************************************/
/* Dashboard */
/************************************************************************/
Dashboard::Dashboard(const WorkflowMonitor *monitor, const QString &_name, QWidget *parent)
: QWebView(parent), name(_name), opened(true), _monitor(monitor), initialized(false)
{
    connect(this, SIGNAL(loadFinished(bool)), SLOT(sl_loaded(bool)));
    connect(_monitor, SIGNAL(si_report()), SLOT(sl_serialize()));
    connect(_monitor, SIGNAL(si_dirSet(const QString &)), SLOT(sl_setDirectory(const QString &)));
    loadDocument(":U2Designer/html/Dashboard.html");
}

Dashboard::Dashboard(const QString &dirPath, QWidget *parent)
: QWebView(parent), dir(dirPath), opened(true), _monitor(NULL), initialized(false)
{
    connect(this, SIGNAL(loadFinished(bool)), SLOT(sl_loaded(bool)));
    loadDocument(dir + REPORT_SUB_DIR + DB_FILE_NAME);
    loadSettings();
    saveSettings();
}

void Dashboard::sl_setDirectory(const QString &value) {
    dir = value;
    U2OpStatus2Log os;
    saveSettings();
}

void Dashboard::setClosed() {
    opened = false;
    U2OpStatus2Log os;
    saveSettings();
}

QString Dashboard::directory() const {
    return dir;
}

QString Dashboard::getName() const {
    return name;
}

void Dashboard::setName(const QString &value) {
    name = value;
    saveSettings();
}

void Dashboard::loadDocument(const QString &filePath) {
    QFile file(filePath);
    bool opened = file.open(QIODevice::ReadOnly);
    SAFE_POINT(opened, "Can not load " + filePath, );
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString html = stream.readAll();
    file.close();

    page()->mainFrame()->setHtml(html);
}

void Dashboard::sl_loaded(bool ok) {
    CHECK(!initialized, );
    SAFE_POINT(ok, "Loaded with errors", );
    initialized = true;
    page()->mainFrame()->addToJavaScriptWindowObject("agent", new JavascriptAgent(this));

    doc = page()->mainFrame()->documentElement();
    if (NULL != monitor()) {
        OutputFilesWidget *files = new OutputFilesWidget(addWidget(tr("Output Files"), OverviewDashTab, 0), this);
        ResourcesWidget *resource = new ResourcesWidget(addWidget(tr("Workflow Task"), OverviewDashTab, 1), this);
        StatisticsWidget *stat = new StatisticsWidget(addWidget(tr("Common Statistics"), OverviewDashTab, 1), this);

        sl_runStateChanged(false);
        if (!monitor()->getProblems().isEmpty()) {
            sl_addProblemsWidget();
        }

        ParametersWidget *params = new ParametersWidget(addWidget(tr("Parameters"), InputDashTab, 0), this);

        connect(monitor(), SIGNAL(si_runStateChanged(bool)), SLOT(sl_runStateChanged(bool)));
        connect(monitor(), SIGNAL(si_firstProblem()), SLOT(sl_addProblemsWidget()));
    }
}

void Dashboard::sl_addProblemsWidget() {
    ProblemsWidget *problems = new ProblemsWidget(addWidget(tr("Problems"), OverviewDashTab), this);
}

void Dashboard::sl_serialize() {
    QString reportDir = dir + REPORT_SUB_DIR;
    QDir d(reportDir);
    if (!d.exists(reportDir)) {
        bool created = d.mkpath(reportDir);
        if (!created) {
            coreLog.error(tr("Can not create a directory: ") + reportDir);
            return;
        }
    }
    U2OpStatus2Log os;
    serialize(os);
    CHECK_OP(os, );
    saveSettings();
}

void Dashboard::serialize(U2OpStatus &os) {
    QString fileName = dir + REPORT_SUB_DIR + DB_FILE_NAME;
    QFile file(fileName);
    bool opened = file.open(QIODevice::WriteOnly);
    if (!opened) {
        os.setError(tr("Can not open a file for writing: ") + fileName);
        return;
    }
    QString html = page()->mainFrame()->toHtml();
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << html;
    stream.flush();
    file.close();
}

void Dashboard::saveSettings() {
    QSettings s(dir + REPORT_SUB_DIR + SETTINGS_FILE_NAME, QSettings::IniFormat);
    s.setValue(OPENED_SETTING, opened);
    s.setValue(NAME_SETTING, name);
    s.sync();
}

void Dashboard::loadSettings() {
    QSettings s(dir + REPORT_SUB_DIR + SETTINGS_FILE_NAME, QSettings::IniFormat);
    opened = true;
    name = s.value(NAME_SETTING).toString();
}

int Dashboard::containerSize(const QWebElement &insideElt, const QString &name) {
    QWebElement cont = insideElt.findFirst(name);
    SAFE_POINT(!cont.isNull(), "NULL container", 0);
    QWebElementCollection children = cont.findAll(".widget");
    return children.count();
}

QWebElement Dashboard::addWidget(const QString &title, DashboardTab dashTab, int cntNum) {
    // Find the tab
    QString dashTabId;
    if (OverviewDashTab == dashTab) {
        dashTabId = "#overview_tab";
    }
    else if (InputDashTab == dashTab) {
        dashTabId = "#input_tab";
    }
    else if (OutputDashTab == dashTab) {
        dashTabId = "#output_tab";
    }
    else {
        FAIL("Unexpected dashboard tab ID!", QWebElement());
    }
    QWebElement tabContainer = doc.findFirst(dashTabId);
    SAFE_POINT(!tabContainer.isNull(), "Can't find the tab container!", QWebElement());

    // Define the location on the tab
    bool left = true;
    if (0 == cntNum) {
        left = true;
    } else if (1 == cntNum) {
        left = false;
    } else if (containerSize(tabContainer, ".left-container") <= containerSize(tabContainer, ".right-container")) {
        left = true;
    } else {
        left = false;
    }

    // Find a container on the tab depending on the location and insert the widget
    QWebElement insideTabContainer = tabContainer.findFirst(left ? ".left-container" : ".right-container");
    SAFE_POINT(!insideTabContainer.isNull(), "Can't find a container inside a tab!", QWebElement());

    insideTabContainer.appendInside(
        "<div class=\"widget\">"
            "<div class=\"title\"><div class=\"title-content\">" + title + "</div></div>"
            "<div class=\"widget-content\"></div>"
        "</div>");

    QWebElement widget = insideTabContainer.lastChild();
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
JavascriptAgent::JavascriptAgent(Dashboard *_dashboard)
: QObject(_dashboard), dashboard(_dashboard)
{

}

void JavascriptAgent::openUrl(const QString &relative) {
    QString url = absolute(relative);
    Task *t = AppContext::getProjectLoader()->openWithProjectTask(url);
    if (t) {
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
}

void JavascriptAgent::openByOS(const QString &relative) {
    QString url = absolute(relative);
    if (!QFile::exists(url)) {
        QMessageBox::critical((QWidget*)AppContext::getMainWindow()->getQMainWindow(), tr("Error"), tr("The file does not exist"));
        return;
    }
    QDesktopServices::openUrl(QUrl("file:///" + url));
}

QString JavascriptAgent::absolute(const QString &url) {
    if (QFileInfo(url).isAbsolute()) {
        return url;
    }
    return dashboard->directory() + url;
}

/************************************************************************/
/* LoadDashboardsTask */
/************************************************************************/
ScanDashboardsDirTask::ScanDashboardsDirTask()
: Task(tr("Scan dashboards directory"), TaskFlag_None)
{

}

void ScanDashboardsDirTask::run() {
    QDir outDir(WorkflowSettings::getWorkflowOutputDirectory());
    CHECK(outDir.exists(), );

    QFileInfoList dirs = outDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach (const QFileInfo &info, dirs) {
        QString dirPath = info.absoluteFilePath() + "/";
        DashboardInfo dbi(dirPath);
        if (isDashboardDir(dirPath, dbi)) {
            result << dbi;
            if (dbi.opened) {
                openedDashboards << dirPath;
            }
        }
    }
}

bool ScanDashboardsDirTask::isDashboardDir(const QString &dirPath, DashboardInfo &info) {
    QDir dir(dirPath + REPORT_SUB_DIR);
    CHECK(dir.exists(), false);
    CHECK(dir.exists(DB_FILE_NAME), false);
    CHECK(dir.exists(SETTINGS_FILE_NAME), false);

    QSettings s(dirPath + REPORT_SUB_DIR + SETTINGS_FILE_NAME, QSettings::IniFormat);
    info.opened = s.value(OPENED_SETTING).toBool();
    info.name = s.value(NAME_SETTING).toString();
    return true;
}

QStringList ScanDashboardsDirTask::getOpenedDashboards() const {
    return openedDashboards;
}

QList<DashboardInfo> ScanDashboardsDirTask::getResult() const {
    return result;
}

/************************************************************************/
/* RemoveDashboardsTask */
/************************************************************************/
RemoveDashboardsTask::RemoveDashboardsTask(const QList<DashboardInfo> &_dashboards)
: Task(tr("Remove dashboards"), TaskFlag_None), dashboards(_dashboards)
{

}
void RemoveDashboardsTask::run() {
    foreach (const DashboardInfo &info, dashboards) {
        U2OpStatus2Log os;
        GUrlUtils::removeDir(info.path, os);
    }
}

/************************************************************************/
/* DashboardInfo */
/************************************************************************/
DashboardInfo::DashboardInfo() {

}

DashboardInfo::DashboardInfo(const QString &dirPath, bool _opened)
: path(dirPath), opened(_opened)
{
    dirName = QDir(path).dirName();
}

bool DashboardInfo::operator==(const DashboardInfo &other) const {
    return path == other.path;
}

} // U2
