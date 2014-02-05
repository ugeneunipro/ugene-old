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

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
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
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Formats/BAMUtils.h>
#include <U2Formats/ConvertFileTask.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "ConvertFilesFormatWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString ConvertFilesFormatWorkerFactory::ACTOR_ID("files-conversion");
static const QString SHORT_NAME( "cff" );
static const QString INPUT_PORT( "in-file" );
static const QString OUTPUT_PORT( "out-file" );
static const QString OUTPUT_SUBDIR( "Converted files/" );
static const QString EXCLUDED_FORMATS_ID( "excluded-formats" );

/************************************************************************/
/* ConvertFilesFormatPrompter */
/************************************************************************/
QString ConvertFilesFormatPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(INPUT_PORT));
    const Actor* producer = input->getProducer(BaseSlots::URL_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);

    QString doc = tr("Convert file %1 to selected format if it is not excluded.").arg(producerName);
    return doc;
}

/************************************************************************/
/* ConvertFilesFormatWorkerFactory */
/************************************************************************/
namespace {
    enum MapType {IDS, BOOLEANS};
    QVariantMap getFormatsMap(MapType mapType) {
        const QList<DocumentFormatId> allFormats = AppContext::getDocumentFormatRegistry()->getRegisteredFormats();

        QVariantMap result;
        foreach (const DocumentFormatId &fid, allFormats) {
            const DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(fid);
            if (NULL == format) {
                continue;
            }
            if (format->getFlags().testFlag(DocumentFormatFlag_SupportWriting) || (BOOLEANS == mapType)) {
                if (BOOLEANS == mapType) {
                    result[fid] = false;
                } else {
                    result[fid] = fid;
                }
            }
        }
        return result;
    }
}

void ConvertFilesFormatWorkerFactory::init() {
    Descriptor desc( ACTOR_ID, ConvertFilesFormatWorker::tr("File Format Conversion"),
        ConvertFilesFormatWorker::tr("Converts the file to selected format if it is not excluded.") );

    QList<PortDescriptor*> p;
    {
        Descriptor inD(INPUT_PORT, ConvertFilesFormatWorker::tr("File"),
            ConvertFilesFormatWorker::tr("A file to perform format conversion"));
        Descriptor outD(OUTPUT_PORT, ConvertFilesFormatWorker::tr("File"),
            ConvertFilesFormatWorker::tr("File of selected format"));

        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(inD, DataTypePtr(new MapDataType(SHORT_NAME + ".input-url", inM)), true);

        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(outD, DataTypePtr(new MapDataType(SHORT_NAME + ".output-url", outM)), false, true);
    }

    QList<Attribute*> a;
    {
        Descriptor excludedFormats(EXCLUDED_FORMATS_ID, ConvertFilesFormatWorker::tr("Excluded formats"),
            ConvertFilesFormatWorker::tr("Input file won't be converted to any of selected formats."));

        a << new Attribute( BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), true );
        a << new Attribute( excludedFormats, BaseTypes::STRING_TYPE(), false );
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap formatsIds = getFormatsMap(IDS);
        delegates[BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId()] = new ComboBoxDelegate(formatsIds);

        QVariantMap formatsBooleans = getFormatsMap(BOOLEANS);
        delegates[EXCLUDED_FORMATS_ID] = new ComboBoxWithChecksDelegate(formatsBooleans);
    }

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new ConvertFilesFormatPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CONVERTERS(), proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new ConvertFilesFormatWorkerFactory());
}

/************************************************************************/
/* ConvertFilesFormatWorker */
/************************************************************************/
ConvertFilesFormatWorker::ConvertFilesFormatWorker(Actor *a)
: BaseWorker(a), inputUrlPort(NULL), outputUrlPort(NULL)
{

}

void ConvertFilesFormatWorker::init() {
    inputUrlPort = ports.value(INPUT_PORT);
    outputUrlPort = ports.value(OUTPUT_PORT);
    targetFormat = getValue<QString>(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
    excludedFormats = getValue<QString>(EXCLUDED_FORMATS_ID).split(",", QString::SkipEmptyParts);
}

Task * ConvertFilesFormatWorker::tick() {
    if (inputUrlPort->hasMessage()) {
        const QString url = takeUrl();
        CHECK(!url.isEmpty(), NULL);

        const QString detectedFormat = detectFormat(url);
        CHECK(!url.isEmpty(), NULL);

        // without conversion
        if ((targetFormat == detectedFormat) || (excludedFormats.contains(detectedFormat))) {
            sendResult(url);
            return NULL;
        }

        Task *t = getConvertTask(detectedFormat, url);
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
        return t;
    } else if (inputUrlPort->isEnded()) {
        setDone();
        outputUrlPort->setEnded();
    }
    return NULL;
}

void ConvertFilesFormatWorker::cleanup() {

}

namespace {
    QString getTargetUrl(Task *task) {
        BamSamConversionTask *bamSamTask = dynamic_cast<BamSamConversionTask*>(task);
        ConvertFileTask *convertFileTask = dynamic_cast<ConvertFileTask*>(task);

        if (NULL != bamSamTask) {
            return bamSamTask->getDestinationURL();
        } else if (NULL != convertFileTask) {
            return convertFileTask->getResult();
        }
        return "";
    }
}

void ConvertFilesFormatWorker::sl_taskFinished(Task *task) {
    CHECK(!task->hasError(), );
    CHECK(!task->isCanceled(), );

    QString url = getTargetUrl(task);
    CHECK(!url.isEmpty(), );

    sendResult(url);
    monitor()->addOutputFile(url, getActorId());
}

QString ConvertFilesFormatWorker::createWorkingDir() {
    QString result = context->workingDir();
    if (!result.endsWith("/")) {
        result += "/";
    }
    result += OUTPUT_SUBDIR;

    QDir dir(result);
    if (!dir.exists(result)) {
        dir.mkdir(result);
    }
    return result;
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

    bool isBamConversion(const QString &srcFormat, const QString &dstFormat) {
        bool isSrcSam = (srcFormat == BaseDocumentFormats::SAM);
        bool isSrcBam = (srcFormat == BaseDocumentFormats::BAM);
        bool isDstSam = (dstFormat == BaseDocumentFormats::SAM);
        bool isDstBam = (dstFormat == BaseDocumentFormats::BAM);

        return (isSrcSam && isDstBam) || (isSrcBam && isDstSam);
    }
}

QString ConvertFilesFormatWorker::takeUrl() {
    const Message inputMessage = getMessageAndSetupScriptValues(inputUrlPort);
    if (inputMessage.isEmpty()) {
        outputUrlPort->transit();
        return "";
    }

    const QVariantMap data = inputMessage.getData().toMap();
    return data[BaseSlots::URL_SLOT().getId()].toString();
}

QString ConvertFilesFormatWorker::detectFormat(const QString &url) {
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

void ConvertFilesFormatWorker::sendResult(const QString &url) {
    const Message message(BaseTypes::STRING_TYPE(), url);
    outputUrlPort->put(message);
}

Task * ConvertFilesFormatWorker::getConvertTask(const QString &detectedFormat, const QString &url) {
    QString workingDir = createWorkingDir();

    if (isBamConversion(detectedFormat, targetFormat)) {
        bool samToBam = (detectedFormat == BaseDocumentFormats::SAM);
        QString extension = (samToBam) ? ".bam" : ".sam";
        QString destinationURL = workingDir + QFileInfo(url).fileName() + extension;
        destinationURL = GUrlUtils::rollFileName(destinationURL, QSet<QString>());

        return new BamSamConversionTask(url, destinationURL, samToBam);
    } else {
        return new ConvertFileTask(url, targetFormat, workingDir);
    }
}

/************************************************************************/
/* BamSamConversionTask */
/************************************************************************/
BamSamConversionTask::BamSamConversionTask(const GUrl &sourceURL, const GUrl &destinationURL, bool samToBam)
: Task(tr("BAM/SAM conversion task"), TaskFlag_None), sourceURL(sourceURL),
destinationURL(destinationURL), samToBam(samToBam)
{

}

void BamSamConversionTask::run() {
    BAMUtils::ConvertOption options(samToBam);
    if (samToBam) {
        BAMUtils::convertToSamOrBam(sourceURL, destinationURL, options, stateInfo);
    } else {
        BAMUtils::convertToSamOrBam(destinationURL, sourceURL, options, stateInfo);
    }
}

QString BamSamConversionTask::getDestinationURL() {
    return destinationURL.getURLString();
}

} //LocalWorkflow
} //U2
