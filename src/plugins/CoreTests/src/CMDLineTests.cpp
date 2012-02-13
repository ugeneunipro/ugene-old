/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <QtGui/QApplication>
#include <QtCore/QCoreApplication>

#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/CMDLineCoreOptions.h>
#include <U2Lang/WorkflowSettings.h>

#include "CMDLineTests.h"

namespace U2 {

#define COMMON_DATA_DIR_ENV_ID "COMMON_DATA_DIR" 
#define TEMP_DATA_DIR_ENV_ID   "TEMP_DATA_DIR" 
#define CONFIG_FILE_ENV_ID "CONFIG_FILE"
#define CONFIG_PROTOTYPE "PROTOTYPE"

/************************
 * GTest_RunCMDLine
 ************************/
#ifndef _DEBUG
const QString GTest_RunCMDLine::UGENECL_PATH    = "/ugenecl";
#else
const QString GTest_RunCMDLine::UGENECL_PATH    = "/ugenecld";
#endif // _DEBUG

const QString GTest_RunCMDLine::TMP_DATA_DIR_PREFIX  = "!tmp_data_dir!";
const QString GTest_RunCMDLine::COMMON_DATA_DIR_PREFIX = "!common_data_dir!";
const QString GTest_RunCMDLine::CONFIG_FILE_PATH = "!config_file_path!";

void GTest_RunCMDLine::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);
    setUgeneclPath();
    setArgs(el);
    proc = new QProcess(this);
    QString protosPath = env->getVar(COMMON_DATA_DIR_ENV_ID) + "/" +  env->getVar(CONFIG_PROTOTYPE);
    QDir protoDir(protosPath), userScriptsDir(WorkflowSettings::getUserDirectory());
    QStringList filters;
    filters << "*.usa";
    protoDir.setNameFilters(filters);

    QFileInfoList list = protoDir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fIdest = list.at(i);
        QFileInfo fItarget(userScriptsDir.path() + "/" + fIdest.fileName());
        QString destPath = fIdest.absoluteFilePath(), targetPath = fItarget.absoluteFilePath();
        if(!fItarget.exists()){
            QFile::copy(fIdest.absoluteFilePath(), fItarget.absoluteFilePath());
        }else if(fIdest.size() != fItarget.size()){
            QFile::copy(fIdest.absoluteFilePath(), fItarget.absoluteFilePath());
        }
    }
}

void GTest_RunCMDLine::setArgs( const QDomElement & el ) {
    QString commandLine;
    QDomNamedNodeMap map = el.attributes();
    int mapSz = map.length();
    for( int i = 0; i < mapSz; ++i ) {
        QDomNode node = map.item(i);
        if(node.nodeName() == "message"){
            expectedMessage = node.nodeValue();
            continue;
        }
        QString argument = "--" + node.nodeName() + "=" + getVal(node.nodeValue());
         if( argument.startsWith("--task") ) {
            args.prepend(argument);
            commandLine.prepend(argument + " ");
        } else {
            args.append(argument);
            commandLine.append(argument + " ");
        }
    }
    args.append("--log-level-details");
    args.append("--lang=en");
    args.append("--log-no-task-progress");
    commandLine.append(QString(" --log-level-details --lang=en --log-no-task-progress"));
    cmdLog.info(commandLine);
}

QString GTest_RunCMDLine::splitVal(const QString & val, int midSize, const QString & prefix, bool isTmp) {
    QString ret;
    QStringList realVals = val.mid(midSize).split(";");
    foreach(const QString & s, realVals) {
        QString filename = prefix + s;
        ret += filename + ";";
        if(isTmp) {
            tmpFiles << filename;
        }
    }
    return ret.mid(0, ret.size() - 1);
}

QString GTest_RunCMDLine::getVal( const QString & val ) {
    if( val.isEmpty() ) {
        return val;
    }
    if( val.startsWith( COMMON_DATA_DIR_PREFIX ) ) {
        return splitVal(val, COMMON_DATA_DIR_PREFIX.size(), env->getVar( COMMON_DATA_DIR_ENV_ID ) + "/", false);
    }
    if( val.startsWith( TMP_DATA_DIR_PREFIX ) ) {
        return splitVal(val, TMP_DATA_DIR_PREFIX.size(), env->getVar( TEMP_DATA_DIR_ENV_ID ) + "/", true);
    }
    if( val == CONFIG_FILE_PATH ) {
        return env->getVar(COMMON_DATA_DIR_ENV_ID) + "/" + env->getVar(CONFIG_FILE_ENV_ID);
    }
    return val;
}

void GTest_RunCMDLine::setUgeneclPath() {
    ugeneclPath = AppContext::getMainWindow() ? QApplication::applicationDirPath() : QCoreApplication::applicationDirPath();
    assert(!ugeneclPath.isEmpty());
    ugeneclPath += UGENECL_PATH;
}

void GTest_RunCMDLine::prepare() {
    QString argsStr = args.join(" ");
    coreLog.trace( "Starting UGENE with arguments: " + argsStr );
    proc->start( ugeneclPath, args );
}

static const QString ERROR_LABEL_TRY1 = "finished with error";
static QString getErrorMsg(const QString & str) {
    int ind = str.indexOf(ERROR_LABEL_TRY1);
    if(ind != -1) {
        return str.mid(ind + ERROR_LABEL_TRY1.size() );
    }
    return QString();
}

Task::ReportResult GTest_RunCMDLine::report() {
    if( hasError() || isCanceled() ) {
        return ReportResult_Finished;
    }
    assert( proc != NULL );
    if( proc->state() != QProcess::NotRunning ) {
        return ReportResult_CallMeAgain;
    }
    //QProcess::ProcessError err = proc->error();
    QString output(proc->readAllStandardOutput());
    //QByteArray outputErr = proc->readAllStandardError();
    cmdLog.trace(output);
    
    if(!expectedMessage.isEmpty()){
        cmdLog.error(output);
        if(!output.contains(expectedMessage, Qt::CaseSensitive)){
            stateInfo.setError(QString("Expected message not found in output"));
        }
        return ReportResult_Finished;
    }
    
    QString err = getErrorMsg(output);
    if( !err.isEmpty() ) {
        int eofIdx = err.indexOf("\n");
        if (eofIdx>0) {
            err = err.left(eofIdx-1);
        }
        setError( "Process finished with error" + err);
    }
    if (proc->exitStatus() == QProcess::CrashExit) {
        setError("Process is crashed!");
    }
    return ReportResult_Finished;
}

void GTest_RunCMDLine::cleanup() {
    foreach(const QString & file, tmpFiles) {
        QFile::remove(file);
    }
}

/************************
* GTest_RunCMDLine
************************/
QList<XMLTestFactory*> CMDLineTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_RunCMDLine::createFactory());
    return res;
}

} // U2
