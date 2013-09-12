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

#include "WorkflowSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/Settings.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtGui/QColor>
#include <QtGui/QStyleFactory>
#include <U2Core/GUrl.h>

namespace U2 {

#define GRID_STATE                  SETTINGS + "showGrid"
#define SNAP_STATE                  SETTINGS + "snap2rid"
#define LOCK_STATE                  SETTINGS + "monitorRun"
#define DEBUGGER_STATE              SETTINGS + "enableDebugger"
#define FAIL_STATE                  SETTINGS + "failFast"
#define STYLE                       SETTINGS + "style"
#define FONT                        SETTINGS + "font"
#define DIR                         "workflow_settings/path"
#define BG_COLOR                    SETTINGS + "bgcolor"
#define RUN_MODE                    SETTINGS + "runMode"
#define SCRIPT_MODE                 SETTINGS + "scriptMode"
#define RUN_IN_SEPARATE_PROC        SETTINGS + "runInSeparateProcess"
#define EXTERNAL_TOOL_WORKER_PATH   SETTINGS + "externalToolWorkerPath"
#define INCLUDED_WORKER_PATH        SETTINGS + "includedWorkerPath"
#define USE_WORKFLOW_OUTPUT_PATH    SETTINGS + "useWorkflowOutputPath"
#define WORKFLOW_OUTPUT_PATH        SETTINGS + "workflowOutputPath"
#define SHOW_LOAD_BUTTON_HINT       SETTINGS + "showLoadButtonHint"

Watcher* const WorkflowSettings::watcher = new Watcher;

bool WorkflowSettings::showGrid() {
    return AppContext::getSettings()->getValue(GRID_STATE, true).toBool();
}

void WorkflowSettings::setShowGrid( bool v ) {
    if (showGrid() != v) {
        AppContext::getSettings()->setValue(GRID_STATE, v);
        emit watcher->changed();
    }
}

bool WorkflowSettings::snap2Grid() {
    return AppContext::getSettings()->getValue(SNAP_STATE, true).toBool();
}

void WorkflowSettings::setSnap2Grid( bool v ) {
    AppContext::getSettings()->setValue(SNAP_STATE, v);
}

bool WorkflowSettings::monitorRun() {
    return AppContext::getSettings()->getValue(LOCK_STATE, true).toBool();
}

void WorkflowSettings::setMonitorRun( bool v ) {
    AppContext::getSettings()->setValue(LOCK_STATE, v);
}

bool WorkflowSettings::isDebuggerEnabled() {
    return AppContext::getSettings()->getValue(DEBUGGER_STATE, false).toBool();
}

void WorkflowSettings::setDebuggerEnabled(bool v) {
    AppContext::getSettings()->setValue(DEBUGGER_STATE, v);
}

/*bool WorkflowSettings::failFast() {
    return AppContext::getSettings()->getValue(FAIL_STATE, true).toBool();
}

void WorkflowSettings::setFailFast( bool v ) {
    AppContext::getSettings()->setValue(FAIL_STATE, v);
}*/

QString WorkflowSettings::defaultStyle()
{
    return AppContext::getSettings()->getValue(STYLE, "ext").toString();
}

void WorkflowSettings::setDefaultStyle(const QString& s){   
    if(s != defaultStyle()){
        AppContext::getSettings()->setValue(STYLE, s);
        emit watcher->changed();
    }
}

QFont WorkflowSettings::defaultFont()
{
    return AppContext::getSettings()->getValue(FONT, true).value<QFont>();
}

void WorkflowSettings::setDefaultFont(const QFont& f)
{   
    if (defaultFont() != f) {
        AppContext::getSettings()->setValue(FONT, qVariantFromValue(f));
        emit watcher->changed();
    }
}

const QString WorkflowSettings::getUserDirectory() {
    Settings *s = AppContext::getSettings();
    QString defaultPath = QDir::searchPaths( PATH_PREFIX_DATA ).first() + "/workflow_samples/" + "users/";
    QString path = s->getValue(DIR, defaultPath, true).toString();
    return path;
}

void WorkflowSettings::setUserDirectory(const QString &newDir) {
    Settings *s = AppContext::getSettings();
    QString defaultPath = QDir::searchPaths( PATH_PREFIX_DATA ).first() + "/workflow_samples/" + "users/";
    QString path = s->getValue(DIR, defaultPath, true).toString();

    AppContext::getSettings()->setValue(DIR, newDir, true);

    if(path != newDir) {
        QDir dir(path);
        if(!dir.exists()) {
            return;
        }
        dir.setNameFilters(QStringList() << "*.usa");
        QFileInfoList fileList = dir.entryInfoList();
        foreach(const QFileInfo &fileInfo, fileList) {
            QString newFileUrl = newDir + fileInfo.fileName();
            QFile::copy(fileInfo.filePath(), newFileUrl);
        }
    }
}

bool WorkflowSettings::isOutputDirectorySet() {
    Settings *s = AppContext::getSettings();
    return s->contains(USE_WORKFLOW_OUTPUT_PATH);
}

bool WorkflowSettings::isUseWorkflowOutputDirectory() {
    Settings *s = AppContext::getSettings();
    return s->getValue(USE_WORKFLOW_OUTPUT_PATH, true).toBool();
}

void  WorkflowSettings::setUseWorkflowOutputDirectory(bool value) {
    Settings *s = AppContext::getSettings();
    s->setValue(USE_WORKFLOW_OUTPUT_PATH, value);
}

void WorkflowSettings::setWorkflowOutputDirectory(const QString &newDir) {
    Settings *s = AppContext::getSettings();
    s->setValue(WORKFLOW_OUTPUT_PATH, newDir);
}

const QString WorkflowSettings::getWorkflowOutputDirectory() {
    Settings *s = AppContext::getSettings();
    GUrl url(s->fileName());
    QString path = s->getValue(WORKFLOW_OUTPUT_PATH, QDir::homePath() + "/workflow_output/").toString();
    return path;
}

const QString WorkflowSettings::getExternalToolDirectory() {
    Settings *s = AppContext::getSettings();
    GUrl url(s->fileName());
    QString defaultPath = url.dirPath();
    defaultPath += "/ExternalToolConfig/";
    QString path = s->getValue(EXTERNAL_TOOL_WORKER_PATH, defaultPath, true).toString();
    return path;
}

void WorkflowSettings::setExternalToolDirectory(const QString &newDir) {
    Settings *s = AppContext::getSettings();
    GUrl url(s->fileName());
    QString defaultPath = url.dirPath();
    defaultPath += "/ExternalToolConfig/";
    QString path = s->getValue(EXTERNAL_TOOL_WORKER_PATH, defaultPath, true).toString();

    s->setValue(EXTERNAL_TOOL_WORKER_PATH, newDir, true);

    if(path != newDir) {
        QDir dir(path);
        if(!dir.exists()) {
            return;
        }
        dir.setNameFilters(QStringList() << "*.etc");
        QFileInfoList fileList = dir.entryInfoList();
        foreach(const QFileInfo &fileInfo, fileList) {
            QString newFileUrl = newDir + fileInfo.fileName();
            QFile::copy(fileInfo.filePath(), newFileUrl);
        }
    }
}

QColor WorkflowSettings::getBGColor() {
    Settings *s = AppContext::getSettings();
    QColor ret(Qt::darkCyan);
    ret.setAlpha(200);
    int r,g,b,a;
    ret.getRgb(&r,&g,&b,&a);
    QString defaultColor = QString::number(r) + "," + QString::number(g) + "," + QString::number(b) + "," + QString::number(a);
    QString color = s->getValue(BG_COLOR,defaultColor).toString();
    QStringList lst = color.split(",");
    if(lst.size() != 4) {
        return ret;
    }

    r = lst[0].toInt();
    g = lst[1].toInt();
    b = lst[2].toInt();
    a = lst[3].toInt();
    QColor res(r,g,b,a);
    return res;
}

void  WorkflowSettings::setBGColor(const QColor &color) {
    int r,g,b,a;
    color.getRgb(&r,&g,&b,&a);
    QString newColor = QString::number(r) + "," + QString::number(g) + "," + QString::number(b) + "," + QString::number(a);
    Settings *s = AppContext::getSettings();
    s->setValue(BG_COLOR, newColor);
}

int WorkflowSettings::getRunMode() {
    Settings * s = AppContext::getSettings();
    int ret = 0;
    QString runModeStr = s->getValue(RUN_MODE).value<QString>();
    if( !runModeStr.isEmpty() ) {
        bool ok = false;
        int num = runModeStr.toInt(&ok);
        if(ok && num >= 0) {
            ret = num;
        }
    }
    return ret;
}

void WorkflowSettings::setRunMode(int md) {
    Settings * s = AppContext::getSettings();
    s->setValue(RUN_MODE, QString::number(md));
}

bool WorkflowSettings::getScriptingMode() {
    return AppContext::getSettings()->getValue(SCRIPT_MODE, QVariant(false)).value<bool>();
}

void WorkflowSettings::setScriptingMode(bool md) {
    AppContext::getSettings()->setValue(SCRIPT_MODE, md);
}

bool WorkflowSettings::runInSeparateProcess() {
    
//#ifdef Q_OS_MAC
    return false; //UGENE-1723
//#endif
    if (!AppContext::isGUIMode()) {
        return false; //for command line mode ugene runs workflows in threads
    }

    bool res = AppContext::getSettings()->getValue(RUN_IN_SEPARATE_PROC, QVariant(true)).value<bool>();
    if (res) {
        QString path = getCmdlineUgenePath();
        res = !path.isEmpty();
    }
    return res;
}

void WorkflowSettings::setRunInSeparateProcess(bool m) {
    return AppContext::getSettings()->setValue(RUN_IN_SEPARATE_PROC, m);
}


static QStringList generateCandidatesWithExt(const QString & path) {
    QStringList res;
    res << path;
    res << path + ".exe";
    return res;
}

static QStringList generateCandidates(const QString & prefix) {
    QStringList res;
    res << generateCandidatesWithExt(prefix + "/" + "ugene");
    res << generateCandidatesWithExt(prefix + "/" + "ugened");
    res << generateCandidatesWithExt(prefix + "/" + "ugenecl");
    res << generateCandidatesWithExt(prefix + "/" + "ugenecld");
    return res;
}

static QString lookupCmdlineUgenePath() {
    QString executableDir = AppContext::getWorkingDirectoryPath();
    QStringList candidates(generateCandidates(executableDir));
    foreach(const QString & candidate, candidates) {
        if(QFile::exists(candidate)) {
            return candidate;
        }
    }
    return QString();
}

static bool lookupDone = false;

QString WorkflowSettings::getCmdlineUgenePath() {
    if (lookupDone) {
        return QString();
    }
    QString path = lookupCmdlineUgenePath();
    if (path.isEmpty()) {
        coreLog.info(tr("Command line UGENE path not found, a possibility to run in separate process will be disabled"));
        return QString();
    }
    return path;
}

void WorkflowSettings::setIncludedElementsDirectory(const QString &newDir) {
    AppContext::getSettings()->setValue(INCLUDED_WORKER_PATH, newDir, true);
}

const QString WorkflowSettings::getIncludedElementsDirectory() {
    Settings *s = AppContext::getSettings();
    GUrl url(s->fileName());
    QString defaultPath = url.dirPath();
    defaultPath += "/IncludedWorkers/";
    QString path = s->getValue(INCLUDED_WORKER_PATH, defaultPath, true).toString();
    return path;
}

bool WorkflowSettings::isShowLoadButtonHint() {
    Settings *s = AppContext::getSettings();
    SAFE_POINT(NULL != s, "NULL settings!", false);

    return s->getValue(SHOW_LOAD_BUTTON_HINT, QVariant(true)).toBool();
}

void WorkflowSettings::setShowLoadButtonHint(bool value) {
    Settings *s = AppContext::getSettings();
    SAFE_POINT(NULL != s, "NULL settings!", );

    s->setValue(SHOW_LOAD_BUTTON_HINT, value);
}

}//namespace
