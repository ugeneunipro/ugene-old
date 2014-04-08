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

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/FailTask.h>
#include <U2Core/GObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Formats/BAMUtils.h>
#include <U2Formats/FileAndDirectoryUtils.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "BedtoolsSupport.h"
#include "BaseBedToolsWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString BaseBedToolsWorker::INPUT_PORT = "in-file";
const QString BaseBedToolsWorker::OUTPUT_PORT = "out-file";
const QString BaseBedToolsWorker::OUT_MODE_ID = "out-mode";
const QString BaseBedToolsWorker::CUSTOM_DIR_ID = "custom-dir";
const QString BaseBedToolsWorker::OUT_NAME_ID = "out-name";
const QString BaseBedToolsWorker::DEFAULT_NAME = "Default";


//////////////////////////////////////////////////////////////////////////
//BaseBedToolsWorker
BaseBedToolsWorker::BaseBedToolsWorker(Actor *a)
:BaseWorker(a)
,inputUrlPort(NULL)
,outputUrlPort(NULL)
,outUrls("")
{

}

void BaseBedToolsWorker::init() {
    inputUrlPort = ports.value(INPUT_PORT);
    outputUrlPort = ports.value(OUTPUT_PORT);
}

Task * BaseBedToolsWorker::tick() {
    if (inputUrlPort->hasMessage()) {
        const QString url = takeUrl();
        CHECK(!url.isEmpty(), NULL);

        const QString outputDir = FileAndDirectoryUtils::createWorkingDir(url, getValue<int>(OUT_MODE_ID), getValue<QString>(CUSTOM_DIR_ID), context->workingDir());

        BedToolsSetting setting;
        setting.outDir = outputDir;
        setting.outName = getTargetName(url, outputDir);
        setting.inputUrl = url;
        setting.customParameters = getCustomParameters();

        Task *t = getTask(setting);
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
        return t;
    }

    if (inputUrlPort->isEnded()) {
        setDone();
        outputUrlPort->setEnded();
    }
    return NULL;
}

void BaseBedToolsWorker::cleanup() {
    outUrls.clear();
}

namespace {
    QString getTargetUrl(Task *task) {

        BaseBedToolsTask *bedToolsTask = dynamic_cast<BaseBedToolsTask*>(task);

        if (NULL != bedToolsTask) {
            return bedToolsTask->getResult();
        }
        return "";
    }
}

void BaseBedToolsWorker::sl_taskFinished(Task *task) {
    CHECK(!task->hasError(), );
    CHECK(!task->isCanceled(), );

    QString url = getTargetUrl(task);
    CHECK(!url.isEmpty(), );

    sendResult(url);
    monitor()->addOutputFile(url, getActorId());
}

QString BaseBedToolsWorker::getTargetName (const QString &fileUrl, const QString &outDir){
    QString name = getValue<QString>(OUT_NAME_ID);

    if(name == DEFAULT_NAME || name.isEmpty()){
        name = QFileInfo(fileUrl).fileName();
        name = name + getDefaultFileName();
    }
    if(outUrls.contains(outDir + name)){
        name.append(QString("_%1").arg(outUrls.size()));
    }
    outUrls.append(outDir+name);
    return name;
}


QString BaseBedToolsWorker::takeUrl() {
    const Message inputMessage = getMessageAndSetupScriptValues(inputUrlPort);
    if (inputMessage.isEmpty()) {
        outputUrlPort->transit();
        return "";
    }

    const QVariantMap data = inputMessage.getData().toMap();
    return data[BaseSlots::URL_SLOT().getId()].toString();
}

void BaseBedToolsWorker::sendResult(const QString &url) {
    const Message message(BaseTypes::STRING_TYPE(), url);
    outputUrlPort->put(message);
}

//////////////////////////////////////////////////////////////////////////
//BaseBedToolsParser
BaseBedToolsParser::BaseBedToolsParser()
    :ExternalToolLogParser() {

}

void BaseBedToolsParser::parseOutput( const QString& partOfLog ){
    ExternalToolLogParser::parseOutput(partOfLog);
}

void BaseBedToolsParser::parseErrOutput( const QString& partOfLog ){
    lastPartOfLog=partOfLog.split(QRegExp("(\n|\r)"));
    lastPartOfLog.first()=lastErrLine+lastPartOfLog.first();
    lastErrLine=lastPartOfLog.takeLast();
    foreach(QString buf, lastPartOfLog){
            if(buf.contains("ERROR", Qt::CaseInsensitive)){
                    coreLog.error("Bedtools: " + buf);
            }
    }
}


//////////////////////////////////////////////////////////////////////////
//BaseBedToolsTask
BaseBedToolsTask::BaseBedToolsTask(const BedToolsSetting &settings)
:Task(QString("Bedtools for %1").arg(settings.inputUrl), TaskFlags_FOSE_COSC)
,settings(settings)
{

}

void BaseBedToolsTask::prepare(){
    if (settings.inputUrl.isEmpty()){
        setError(tr("No input URL"));
        return ;
    }

    const QDir outDir = QFileInfo(settings.outDir).absoluteDir();
    if (!outDir.exists()) {
        setError(tr("Directory does not exist: ") + outDir.absolutePath());
        return ;
    }

    const QStringList args = getParameters(stateInfo);
    CHECK_OP(stateInfo, );

    ExternalToolLogParser* logParser = new BaseBedToolsParser();
    ExternalToolRunTask* etTask = new ExternalToolRunTask(ET_BEDTOOLS, args, logParser, settings.outDir);
    etTask->setStandartOutputFile(settings.outDir + settings.outName);
    addSubTask(etTask);
}

void BaseBedToolsTask::run(){
    CHECK_OP(stateInfo, );

    resultUrl = settings.outDir + settings.outName;
}

} //LocalWorkflow
} //U2
