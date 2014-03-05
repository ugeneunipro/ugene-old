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

#include <QDesktopServices>
#include <QFile>


#if (QT_VERSION < 0x050000) //Qt 5
#include <QMessageBox>
#include <QWebFrame>
#include <QtGui/QApplication>
#else
#include <QtWidgets/QMessageBox>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWidgets/QApplication>
#endif
#include <QClipboard>

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
#include "ExternalToolsWidget.h"

#include "Dashboard.h"

namespace U2 {

static const QString REPORT_SUB_DIR("report/");
static const QString DB_FILE_NAME("dashboard.html");
static const QString SETTINGS_FILE_NAME("settings.ini");
static const QString SCHEMA_FILE_NAME("schema.uwl");
static const QString OPENED_SETTING("opened");
static const QString NAME_SETTING("name");

/************************************************************************/
/* Dashboard */
/************************************************************************/
const QString Dashboard::EXT_TOOLS_TAB_ID = "#ext_tools_tab";
const QString Dashboard::OVERVIEW_TAB_ID = "#overview_tab";
const QString Dashboard::INPUT_TAB_ID = "#input_tab";
//const QString Dashboard::OUTPUT_TAB_ID = "#output_tab";

Dashboard::Dashboard(const WorkflowMonitor *monitor, const QString &_name, QWidget *parent)
: QWebView(parent), loaded(false), name(_name), opened(true), _monitor(monitor), initialized(false)
{
    etWidgetController = new ExternalToolsWidgetController;

    connect(this, SIGNAL(loadFinished(bool)), SLOT(sl_loaded(bool)));
    connect(_monitor, SIGNAL(si_report()), SLOT(sl_serialize()));
    connect(_monitor, SIGNAL(si_dirSet(const QString &)), SLOT(sl_setDirectory(const QString &)));
    connect(_monitor, SIGNAL(si_logChanged(U2::Workflow::Monitor::LogEntry)),
            etWidgetController, SLOT(sl_onLogChanged(U2::Workflow::Monitor::LogEntry)));

    setContextMenuPolicy(Qt::NoContextMenu);
    loadUrl = ":U2Designer/html/Dashboard.html";
    loadDocument();
}

Dashboard::Dashboard(const QString &dirPath, QWidget *parent)
: QWebView(parent), loaded(false), dir(dirPath), opened(true), _monitor(NULL), initialized(false)
{
    etWidgetController = new ExternalToolsWidgetController;

    connect(this, SIGNAL(loadFinished(bool)), SLOT(sl_loaded(bool)));
    setContextMenuPolicy(Qt::NoContextMenu);
    loadUrl = dir + REPORT_SUB_DIR + DB_FILE_NAME;
    loadSettings();
    saveSettings();
}

Dashboard::~Dashboard() {
    delete etWidgetController;
}

void Dashboard::onShow() {
    CHECK(!loaded, );
    loadDocument();
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

void Dashboard::loadDocument() {
    loaded = true;
    QFile file(loadUrl);
    bool opened = file.open(QIODevice::ReadOnly);
    if (!opened) {
        coreLog.error("Can not load " + loadUrl);
        return;
    }

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
        new OutputFilesWidget(addWidget(tr("Output Files"), OverviewDashTab, 0), this);
        new ResourcesWidget(addWidget(tr("Workflow Task"), OverviewDashTab, 1), this);
        new StatisticsWidget(addWidget(tr("Common Statistics"), OverviewDashTab, 1), this);

        sl_runStateChanged(false);
        if (!monitor()->getProblems().isEmpty()) {
            sl_addProblemsWidget();
        }

        new ParametersWidget(addWidget(tr("Parameters"), InputDashTab, 0), this);

        //new OutputFilesWidget(addWidget(tr("Output Files"), OutputDashTab, 0), this);

        createExternalToolTab();

        connect(monitor(), SIGNAL(si_runStateChanged(bool)), SLOT(sl_runStateChanged(bool)));
        connect(monitor(), SIGNAL(si_firstProblem()), SLOT(sl_addProblemsWidget()));
    }

    if (!WorkflowSettings::isShowLoadButtonHint()) {
        page()->mainFrame()->documentElement().evaluateJavaScript("hideLoadBtnHint()");
    }
}

void Dashboard::sl_addProblemsWidget() {
    // Will be removed by parent
    new ProblemsWidget(addWidget(tr("Problems"), OverviewDashTab), this);
}

void Dashboard::sl_serialize() {
    QCoreApplication::processEvents();
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

void Dashboard::createExternalToolTab() {
    SAFE_POINT(etWidgetController, "External tools widget controller is NULL", );
    const WorkflowMonitor* mon = monitor();
    SAFE_POINT(mon, "Monitor is NULL", );

    foreach (const WorkerParamsInfo& info, mon->getWorkersParameters()) {
        SAFE_POINT(info.actor, "Actor is NULL", );
        const ActorPrototype* proto = info.actor->getProto();
        SAFE_POINT(proto, "Actor prototype is NULL", );

        if (!proto->getExternalTools().isEmpty()) {
            QString addTabJs = "addTab('" + EXT_TOOLS_TAB_ID + "','" + tr("External Tools") + "')";

            QWebPage* mainPage = page();
            SAFE_POINT(mainPage, "Page is NULL", );
            QWebFrame* mainFrame = mainPage->mainFrame();
            SAFE_POINT(mainFrame, "Main frame is NULL", );

            mainFrame->documentElement().evaluateJavaScript(addTabJs);
            etWidgetController->getWidget(addWidget(tr("External Tools"), ExternalToolsTab, 0), this);
            break;
        }
    }
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
        dashTabId = OVERVIEW_TAB_ID;
    }
    else if (InputDashTab == dashTab) {
        dashTabId = INPUT_TAB_ID;
    }
    /*else if (OutputDashTab == dashTab) {
        dashTabId = OUTPUT_TAB_ID;
    }*/
    else if (ExternalToolsTab == dashTab) {
        dashTabId = EXT_TOOLS_TAB_ID;
    }
    else {
        FAIL("Unexpected dashboard tab ID!", QWebElement());
    }
    QWebElement tabContainer = doc.findFirst(dashTabId);
    SAFE_POINT(!tabContainer.isNull(), "Can't find the tab container!", QWebElement());

    // Specify if the tab has left/right inner containers
    bool hasInnerContainers = true;
    if (InputDashTab == dashTab || ExternalToolsTab == dashTab) {
        hasInnerContainers = false;
    }

    // Get the left or right inner container (if the tab allows),
    // otherwise use the whole tab as a container
    QWebElement mainContainer = tabContainer;

    if (hasInnerContainers) {
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

        mainContainer = tabContainer.findFirst(left ? ".left-container" : ".right-container");
        SAFE_POINT(!mainContainer.isNull(), "Can't find a container inside a tab!", QWebElement());
    }

    mainContainer.appendInside(
        "<div class=\"widget\">"
            "<div class=\"title\"><div class=\"title-content\">" + title + "</div></div>"
            "<div class=\"widget-content\"></div>"
        "</div>");

    QWebElement widget = mainContainer.lastChild();
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

void Dashboard::loadSchema() {
    QString url = dir + REPORT_SUB_DIR + SCHEMA_FILE_NAME;
    emit si_loadSchema(url);
}

void Dashboard::initiateHideLoadButtonHint() {
    WorkflowSettings::setShowLoadButtonHint(false);
    emit si_hideLoadBtnHint();
}

void Dashboard::sl_hideLoadBtnHint() {
    page()->mainFrame()->evaluateJavaScript("hideLoadBtnHint()");
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

void JavascriptAgent::loadSchema() {
    dashboard->loadSchema();
}

void JavascriptAgent::hideLoadButtonHint() {
    SAFE_POINT(NULL != dashboard, "NULL dashboard!", );
    dashboard->initiateHideLoadButtonHint();
}
void JavascriptAgent::setClipboardText(const QString &text) {
    QApplication::clipboard()->setText(text);
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
