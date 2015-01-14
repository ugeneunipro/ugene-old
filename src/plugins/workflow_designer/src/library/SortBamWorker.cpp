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
#include <U2Core/FileAndDirectoryUtils.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "SortBamWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString SortBamWorkerFactory::ACTOR_ID("Sort-bam");
static const QString SHORT_NAME( "mb" );
static const QString INPUT_PORT( "in-file" );
static const QString OUTPUT_PORT( "out-file" );
static const QString OUT_MODE_ID( "out-mode" );
static const QString CUSTOM_DIR_ID( "custom-dir" );
static const QString OUT_NAME_ID( "out-name" );
static const QString INDEX_ID( "index" );

/************************************************************************/
/* SortBamPrompter */
/************************************************************************/
QString SortBamPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(INPUT_PORT));
    const Actor* producer = input->getProducer(BaseSlots::URL_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);

    QString doc = tr("Sort BAM file %1 with SAMTools sort.").arg(producerName);
    return doc;
}

/************************************************************************/
/* SortBamWorkerFactory */
/************************************************************************/
namespace {
    static const QString DEFAULT_NAME( "Default" );
}

void SortBamWorkerFactory::init() {
    Descriptor desc( ACTOR_ID, SortBamWorker::tr("Sort BAM Files"),
        SortBamWorker::tr("Sort BAM Files using SAMTools Sort.") );

    QList<PortDescriptor*> p;
    {
        Descriptor inD(INPUT_PORT, SortBamWorker::tr("BAM File"),
            SortBamWorker::tr("Set of BAM files to sort"));
        Descriptor outD(OUTPUT_PORT, SortBamWorker::tr("Sorted BAM File"),
            SortBamWorker::tr("Sorted BAM file"));

        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(inD, DataTypePtr(new MapDataType(SHORT_NAME + ".input-url", inM)), true);

        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(outD, DataTypePtr(new MapDataType(SHORT_NAME + ".output-url", outM)), false, true);
    }

    QList<Attribute*> a;
    {
        Descriptor outDir(OUT_MODE_ID, SortBamWorker::tr("Output directory"),
            SortBamWorker::tr("Select an output directory. <b>Custom</b> - specify the output directory in the 'Custom directory' parameter. "
            "<b>Workflow</b> - internal workflow directory. "
            "<b>Input file</b> - the directory of the input file."));

        Descriptor customDir(CUSTOM_DIR_ID, SortBamWorker::tr("Custom directory"),
            SortBamWorker::tr("Select the custom output directory."));

        Descriptor outName(OUT_NAME_ID, SortBamWorker::tr("Output BAM name"),
            SortBamWorker::tr("A name of an output BAM file. If default of empty value is provided the output name is the name of the first BAM file with .sorted.bam extention."));

        Descriptor index(INDEX_ID, SortBamWorker::tr("Build index"),
            SortBamWorker::tr("Build index for the sorted file with SAMTools index."));

        a << new Attribute( outDir, BaseTypes::NUM_TYPE(), false, QVariant(FileAndDirectoryUtils::FILE_DIRECTORY));
        Attribute* customDirAttr = new Attribute(customDir, BaseTypes::STRING_TYPE(), false, QVariant(""));
        customDirAttr->addRelation(new VisibilityRelation(OUT_MODE_ID, FileAndDirectoryUtils::CUSTOM));
        a << customDirAttr;
        a << new Attribute( outName, BaseTypes::STRING_TYPE(), false, QVariant(DEFAULT_NAME));
        a << new Attribute( index, BaseTypes::BOOL_TYPE(), false, QVariant(true));
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap directoryMap;
        QString fileDir = SortBamWorker::tr("Input file");
        QString workflowDir = SortBamWorker::tr("Workflow");
        QString customD = SortBamWorker::tr("Custom");
        directoryMap[fileDir] = FileAndDirectoryUtils::FILE_DIRECTORY;
        directoryMap[workflowDir] = FileAndDirectoryUtils::WORKFLOW_INTERNAL;
        directoryMap[customD] = FileAndDirectoryUtils::CUSTOM;
        delegates[OUT_MODE_ID] = new ComboBoxDelegate(directoryMap);

        delegates[CUSTOM_DIR_ID] = new URLDelegate("", "", false, true);
    }

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new SortBamPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_NGS_BASIC(), proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new SortBamWorkerFactory());
}

/************************************************************************/
/* SortBamWorker */
/************************************************************************/
SortBamWorker::SortBamWorker(Actor *a)
:BaseWorker(a)
,inputUrlPort(NULL)
,outputUrlPort(NULL)
,outUrls("")
{

}

void SortBamWorker::init() {
    inputUrlPort = ports.value(INPUT_PORT);
    outputUrlPort = ports.value(OUTPUT_PORT);
}

Task * SortBamWorker::tick() {
    if (inputUrlPort->hasMessage()) {
        const QString url = takeUrl();
        CHECK(!url.isEmpty(), NULL);

        const QString detectedFormat = FileAndDirectoryUtils::detectFormat(url);
        if(detectedFormat.isEmpty()){
            coreLog.info(tr("Unknown file format: ") + url);
            return NULL;
        }

        if(detectedFormat == BaseDocumentFormats::BAM){
            const QString outputDir = FileAndDirectoryUtils::createWorkingDir(url, getValue<int>(OUT_MODE_ID), getValue<QString>(CUSTOM_DIR_ID), context->workingDir());

            BamSortSetting setting;
            setting.outDir = outputDir;
            setting.outName = getTargetName(url, outputDir);
            setting.inputUrl = url;
            setting.index = getValue<bool>(INDEX_ID);

            Task *t = new SamtoolsSortTask(setting);
            connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
            return t;
        }
    }

    if (inputUrlPort->isEnded()) {
        setDone();
        outputUrlPort->setEnded();
    }
    return NULL;
}

void SortBamWorker::cleanup() {
    outUrls.clear();
}

namespace {
    QString getTargetUrl(Task *task) {

        SamtoolsSortTask *sortTask = dynamic_cast<SamtoolsSortTask*>(task);

        if (NULL != sortTask) {
            return sortTask->getResult();
        }
        return "";
    }
}

void SortBamWorker::sl_taskFinished(Task *task) {
    CHECK(!task->hasError(), );
    CHECK(!task->isCanceled(), );

    QString url = getTargetUrl(task);
    CHECK(!url.isEmpty(), );

    sendResult(url);
    monitor()->addOutputFile(url, getActorId());
}

QString SortBamWorker::getTargetName (const QString &fileUrl, const QString &outDir){
    QString name = getValue<QString>(OUT_NAME_ID);

    if(name == DEFAULT_NAME || name.isEmpty()){
        name = QFileInfo(fileUrl).fileName();
        name = name + ".sorted.bam";
    }
    if(outUrls.contains(outDir + name)){
        name.append(QString("_%1").arg(outUrls.size()));
    }
    outUrls.append(outDir+name);
    return name;
}


QString SortBamWorker::takeUrl() {
    const Message inputMessage = getMessageAndSetupScriptValues(inputUrlPort);
    if (inputMessage.isEmpty()) {
        outputUrlPort->transit();
        return "";
    }

    const QVariantMap data = inputMessage.getData().toMap();
    return data[BaseSlots::URL_SLOT().getId()].toString();
}

void SortBamWorker::sendResult(const QString &url) {
    const Message message(BaseTypes::STRING_TYPE(), url);
    outputUrlPort->put(message);
}

SamtoolsSortTask::SamtoolsSortTask(const BamSortSetting &settings)
:Task(QString("Samtools sort for %1").arg(settings.inputUrl), TaskFlag_None)
,settings(settings)
{

}

void SamtoolsSortTask::prepare(){
    if (settings.inputUrl.isEmpty()){
        setError(tr("No assembly URL to filter"));
        return ;
    }

    const QDir outDir = QFileInfo(settings.outDir).absoluteDir();
    if (!outDir.exists()) {
        setError(tr("Directory does not exist: ") + outDir.absolutePath());
        return ;
    }
}

void SamtoolsSortTask::run(){
    resultUrl = BAMUtils::sortBam(settings.inputUrl, settings.outDir + settings.outName, stateInfo).getURLString();
    CHECK_OP(stateInfo, );

    BAMUtils::createBamIndex(resultUrl, stateInfo);
}

} //LocalWorkflow
} //U2
