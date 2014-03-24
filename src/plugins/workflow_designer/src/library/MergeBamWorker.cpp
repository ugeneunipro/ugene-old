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
#include <U2Formats/MergeBamTask.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "MergeBamWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString MergeBamWorkerFactory::ACTOR_ID("merge-bam");
static const QString SHORT_NAME( "mb" );
static const QString INPUT_PORT( "in-file" );
static const QString OUTPUT_PORT( "out-file" );
static const QString OUTPUT_SUBDIR( "Merged_BAM/" );
static const QString OUT_MODE_ID( "out-mode" );
static const QString CUSTOM_DIR_ID( "custom-dir" );
static const QString OUT_NAME_ID( "out-name" );

/************************************************************************/
/* MergeBamPrompter */
/************************************************************************/
QString MergeBamPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(INPUT_PORT));
    const Actor* producer = input->getProducer(BaseSlots::URL_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);

    QString doc = tr("Merge BAM files from %1 with SAMTools merge.").arg(producerName);
    return doc;
}

/************************************************************************/
/* MergeBamWorkerFactory */
/************************************************************************/
namespace {
    enum OutDirectory{
        FILE_DIRECTORY = 0,
        WORKFLOW_INTERNAL,
        CUSTOM
    };

    static const QString DEFAULT_NAME( "Default" );
}

void MergeBamWorkerFactory::init() {
    Descriptor desc( ACTOR_ID, MergeBamWorker::tr("Merge BAM files"),
        MergeBamWorker::tr("Merge BAM files using SAMTools merge.") );

    QList<PortDescriptor*> p;
    {
        Descriptor inD(INPUT_PORT, MergeBamWorker::tr("BAM File"),
            MergeBamWorker::tr("Set of BAM files to merge"));
        Descriptor outD(OUTPUT_PORT, MergeBamWorker::tr("Merged BAM File"),
            MergeBamWorker::tr("Merged BAM file"));

        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(inD, DataTypePtr(new MapDataType(SHORT_NAME + ".input-url", inM)), true);

        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(outD, DataTypePtr(new MapDataType(SHORT_NAME + ".output-url", outM)), false, true);
    }

    QList<Attribute*> a;
    {
        Descriptor outDir(OUT_MODE_ID, MergeBamWorker::tr("Output directory"),
            MergeBamWorker::tr("Select an output directory. <b>Custom</b> - specify the output directory in the 'Custom directory' parameter. "
            "<b>Workflow</b> - internal workflow directory. "
            "<b>Input file</b> - the directory of the input file."));

        Descriptor customDir(CUSTOM_DIR_ID, MergeBamWorker::tr("Custom directory"),
            MergeBamWorker::tr("Select the custom output directory."));

        Descriptor outName(OUT_NAME_ID, MergeBamWorker::tr("Output BAM name"),
            MergeBamWorker::tr("A name of an output BAM file. If default of empty value is provided the output name is the name of the first BAM file with .merged.bam extention."));


        a << new Attribute( outDir, BaseTypes::NUM_TYPE(), false, QVariant(FILE_DIRECTORY));
        Attribute* customDirAttr = new Attribute(customDir, BaseTypes::STRING_TYPE(), false, QVariant(""));
        customDirAttr->addRelation(new VisibilityRelation(OUT_MODE_ID, MergeBamWorker::tr("Custom")));
        a << customDirAttr;
        a << new Attribute( outName, BaseTypes::STRING_TYPE(), false, QVariant(DEFAULT_NAME));
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap directoryMap;
        QString fileDir = MergeBamWorker::tr("Input file");
        QString workflowDir = MergeBamWorker::tr("Workflow");
        QString customD = MergeBamWorker::tr("Custom");
        directoryMap[fileDir] = FILE_DIRECTORY;
        directoryMap[workflowDir] = WORKFLOW_INTERNAL;
        directoryMap[customD] = CUSTOM;
        delegates[OUT_MODE_ID] = new ComboBoxDelegate(directoryMap);

        delegates[CUSTOM_DIR_ID] = new URLDelegate("", "", false, true);
    }

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new MergeBamPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CONVERTERS(), proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new MergeBamWorkerFactory());
}

/************************************************************************/
/* MergeBamWorker */
/************************************************************************/
MergeBamWorker::MergeBamWorker(Actor *a)
:BaseWorker(a)
,inputUrlPort(NULL)
,outputUrlPort(NULL)
,outputDir("")
{

}

void MergeBamWorker::init() {
    inputUrlPort = ports.value(INPUT_PORT);
    outputUrlPort = ports.value(OUTPUT_PORT);
}

Task * MergeBamWorker::tick() {
    while (inputUrlPort->hasMessage()) {
        const QString url = takeUrl();
        CHECK(!url.isEmpty(), NULL);

        const QString detectedFormat = detectFormat(url);
        CHECK(!url.isEmpty(), NULL);

        if(detectedFormat == BaseDocumentFormats::BAM){
            if(outputDir.isEmpty()){
                outputDir = createWorkingDir(url);
            }
            urls.append(url);
        }
    }

    if (!inputUrlPort->isEnded()) {
        return NULL;
    }

    if(!urls.isEmpty()){
        const QString targetName = getOutputName(urls.first());
        CHECK(!targetName.isEmpty(), NULL);

        Task *t = new MergeBamTask(urls, outputDir, targetName);
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
        urls.clear();
        return t;
    }

    if (inputUrlPort->isEnded()) {
        setDone();
        outputUrlPort->setEnded();
    }
    return NULL;
}

void MergeBamWorker::cleanup() {
    outputDir = "";
    urls.clear();
}

namespace {
    QString getTargetUrl(Task *task) {

        MergeBamTask *mergeTask = dynamic_cast<MergeBamTask*>(task);

        if (NULL != mergeTask) {
            return mergeTask->getResult();
        }
        return "";
    }
}

void MergeBamWorker::sl_taskFinished(Task *task) {
    CHECK(!task->hasError(), );
    CHECK(!task->isCanceled(), );

    QString url = getTargetUrl(task);
    CHECK(!url.isEmpty(), );

    sendResult(url);
    monitor()->addOutputFile(url, getActorId());
}

QString MergeBamWorker::createWorkingDir( const QString& fileUrl ){
    QString result;

    bool useInternal = false;

    int dirMode = getValue<int>(OUT_MODE_ID);

    if(dirMode == FILE_DIRECTORY){
        result = GUrl(fileUrl).dirPath() + "/";
    }else if (dirMode == CUSTOM){
        QString customDir = getValue<QString>(CUSTOM_DIR_ID);
        if (!customDir.isEmpty()){
            result = customDir;
            if (!result.endsWith("/")) {
                result += "/";
            }
        }else{
            algoLog.error(tr("Merge BAM: result directory is empty, default workflow directory is used"));
            useInternal = true;
        }
    }else{
        useInternal = true;
    }
    
    if (useInternal){
        result = context->workingDir();
        if (!result.endsWith("/")) {
            result += "/";
        }
        result += OUTPUT_SUBDIR;
    }

    QDir dir(result);
    if (!dir.exists(result)) {
        dir.mkdir(result);
    }
    return result;
}
QString MergeBamWorker::getOutputName (const QString& fileUrl ){
    QString name = getValue<QString>(OUT_NAME_ID);

    if(name == DEFAULT_NAME || name.isEmpty()){
        name = QFileInfo(fileUrl).fileName();
        name = name + ".merged.bam";
    }
    return name;
}


namespace {
    QString getFormatId(const FormatDetectionResult &r) {
        if (NULL != r.format) {
            return r.format->getFormatId();
        }
        if (NULL != r.importer) {
            return r.importer->getId();
        }
        return "";
    }
}

QString MergeBamWorker::takeUrl() {
    const Message inputMessage = getMessageAndSetupScriptValues(inputUrlPort);
    if (inputMessage.isEmpty()) {
        outputUrlPort->transit();
        return "";
    }

    const QVariantMap data = inputMessage.getData().toMap();
    return data[BaseSlots::URL_SLOT().getId()].toString();
}

QString MergeBamWorker::detectFormat(const QString &url) {
    FormatDetectionConfig cfg;
    cfg.bestMatchesOnly = false;
    cfg.useImporters = true;
    cfg.excludeHiddenFormats = false;

    const QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(url, cfg);
    if (formats.empty()) {
        monitor()->addError(tr("Unknown file format: ") + url, getActorId());
        return "";
    }

    return getFormatId(formats.first());
}

void MergeBamWorker::sendResult(const QString &url) {
    const Message message(BaseTypes::STRING_TYPE(), url);
    outputUrlPort->put(message);
}

} //LocalWorkflow
} //U2
