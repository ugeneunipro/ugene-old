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

#include "ExternalProcessWorker.h"

#include <U2Core/AppSettings.h>
#include <U2Core/AppContext.h>
#include <U2Core/FailTask.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/TextObject.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceUtils.h>

#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/ExternalToolCfg.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/IncludedProtoFactory.h>

#include <U2Designer/DelegateEditors.h>

#include <QtCore/QDateTime>

namespace U2 {
namespace LocalWorkflow {

const static QString INPUT_PORT_TYPE("input-for-");
const static QString OUTPUT_PORT_TYPE("output-for-");
static const QString OUT_PORT_ID("out");

bool ExternalProcessWorkerFactory::init(ExternalProcessConfig *cfg) {
    ActorPrototype *proto = IncludedProtoFactory::getExternalToolProto(cfg);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_EXTERNAL(), proto);
    IncludedProtoFactory::registerExternalToolWorker(cfg);
    return true;
}

namespace {
    static QString toStringValue(const QVariantMap &data, U2OpStatus &os) {
        QString slot = BaseSlots::TEXT_SLOT().getId();
        if (!data.contains(slot)) {
            os.setError(QObject::tr("Empty text slot"));
            return "";
        }
        return data[slot].value<QString>();
    }

    static U2SequenceObject * toSequence(const QVariantMap &data, WorkflowContext *context, U2OpStatus &os) {
        QString slot = BaseSlots::DNA_SEQUENCE_SLOT().getId();
        if (!data.contains(slot)) {
            os.setError(QObject::tr("Empty sequence slot"));
            return NULL;
        }
        SharedDbiDataHandler seqId = data[slot].value<SharedDbiDataHandler>();
        U2SequenceObject *seqObj = StorageUtils::getSequenceObject(context->getDataStorage(), seqId);
        if (NULL == seqObj) {
            os.setError(QObject::tr("Error with sequence object"));
        }
        return seqObj;
    }

    static AnnotationTableObject * toAnotations(const QVariantMap &data, WorkflowContext *context, U2OpStatus &os) {
        QString slot = BaseSlots::ANNOTATION_TABLE_SLOT().getId();
        if (!data.contains(slot)) {
            os.setError(QObject::tr("Empty annotations slot"));
            return NULL;
        }
        const QVariant annotationsData = data[slot];
        const QList<SharedAnnotationData> annList = StorageUtils::getAnnotationTable(context->getDataStorage(), annotationsData);

        AnnotationTableObject *annsObj = new AnnotationTableObject("Annotations", context->getDataStorage()->getDbiRef());
        annsObj->addAnnotations(annList);

        return annsObj;
    }

    static MAlignmentObject * toAlignment(const QVariantMap &data, WorkflowContext *context, U2OpStatus &os) {
        QString slot = BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId();
        if (!data.contains(slot)) {
            os.setError(QObject::tr("Empty alignment slot"));
            return NULL;
        }
        SharedDbiDataHandler msaId = data[slot].value<SharedDbiDataHandler>();
        MAlignmentObject *msaObj = StorageUtils::getMsaObject(context->getDataStorage(), msaId);
        if (NULL == msaObj) {
            os.setError(QObject::tr("Error with alignment object"));
        }
        return msaObj;
    }

    static TextObject * toText(const QVariantMap &data, WorkflowContext *context, U2OpStatus &os) {
        QString slot = BaseSlots::TEXT_SLOT().getId();
        if (!data.contains(slot)) {
            os.setError(QObject::tr("Empty text slot"));
            return NULL;
        }
        QString text = data[slot].value<QString>();
        return TextObject::createInstance(text, "tmp_text_object", context->getDataStorage()->getDbiRef(), os);
    }

    static QString generateAndCreateURL(const QString &extention, const QString &name) {
        QString url;
        QString path = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath("wd_external");
        QDir dir(path);
        if (!dir.exists()) {
            dir.mkpath(path);
        }
        url = path + "/tmp" + name + QString::number(QDateTime::currentDateTime().toTime_t()) +  "." + extention;
        return url;
    }

    static DocumentFormat * getFormat(const DataConfig &dataCfg, U2OpStatus &os) {
        DocumentFormat *f = AppContext::getDocumentFormatRegistry()->getFormatById(dataCfg.format);
        if (NULL == f) {
            os.setError(QObject::tr("Unknown document format: %1").arg(dataCfg.format));
        }
        return f;
    }

    static Document * createDocument(const DataConfig &dataCfg, U2OpStatus &os) {
        DocumentFormat *f = getFormat(dataCfg, os);
        CHECK_OP(os, NULL);

        IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        QString url = generateAndCreateURL(f->getSupportedDocumentFileExtensions().first(), dataCfg.attrName);
        QScopedPointer<Document> d(f->createNewLoadedDocument(iof, url, os));
        CHECK_OP(os, NULL);
        d->setDocumentOwnsDbiResources(false);
        return d.take();
    }

    static Document * loadDocument(const QString &url, const DataConfig &dataCfg, WorkflowContext *context, U2OpStatus &os) {
        DocumentFormat *f = getFormat(dataCfg, os);
        CHECK_OP(os, NULL);

        IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        QVariantMap hints;
        U2DbiRef dbiRef = context->getDataStorage()->getDbiRef();
        hints.insert(DocumentFormat::DBI_REF_HINT, qVariantFromValue(dbiRef));
        QScopedPointer<Document> d(f->loadDocument(iof, url, hints, os));
        CHECK_OP(os, NULL);
        d->setDocumentOwnsDbiResources(false);
        return d.take();
    }

    static void addObjects(Document *d, WorkflowContext *context, const DataConfig &dataCfg, const QVariantMap &data, U2OpStatus &os) {
        if (dataCfg.isSequence()) {
            U2SequenceObject *seqObj = toSequence(data, context, os);
            CHECK_OP(os,);
            d->addObject(seqObj);
        } else if (dataCfg.isAnnotations()) {
            AnnotationTableObject *annsObj = toAnotations(data, context, os);
            CHECK_OP(os,);
            d->addObject(annsObj);
        } else if (dataCfg.isAlignment()) {
            MAlignmentObject *msaObj = toAlignment(data, context, os);
            CHECK_OP(os,);
            d->addObject(msaObj);
        } else if (dataCfg.isAnnotatedSequence()) {
            U2SequenceObject *seqObj = toSequence(data, context, os);
            CHECK_OP(os,);
            d->addObject(seqObj);
            AnnotationTableObject *annsObj = toAnotations(data, context, os);
            CHECK_OP(os,);
            d->addObject(annsObj);

            QList<GObjectRelation> rel;
            rel << GObjectRelation(GObjectReference(seqObj), ObjectRole_Sequence);
            annsObj->setObjectRelations(rel);
        } else if (dataCfg.isText()) {
            TextObject *textObj = toText(data, context, os);
            CHECK_OP(os,);
            d->addObject(textObj);
        }
    }
} // namespace

void ExternalProcessWorker::applyAttributes(QString &execString) {
    foreach(Attribute *a, actor->getAttributes()) {
        int pos = execString.indexOf(QRegExp("\\$" + a->getDisplayName() + "(\\W|$)"));
        if (-1 == pos) {
            continue;
        }

        //set parameters in command line with attributes values
        QString value = getValue<QString>(a->getId());
        int idLength = a->getDisplayName().size() + 1;
        execString.replace(pos, idLength, value);
    }
}

QStringList ExternalProcessWorker::applyInputMessage(QString &execString, const DataConfig &dataCfg, const QVariantMap &data, U2OpStatus &os) {
    QStringList urls;
    int ind = execString.indexOf(QRegExp("\\$" + dataCfg.attrName + "(\\W|$)"));
    CHECK(-1 != ind, urls);

    QString paramValue;
    if (dataCfg.isStringValue()) {
        paramValue = toStringValue(data, os);
        CHECK_OP(os, urls);
    } else {
        QScopedPointer<Document> d(createDocument(dataCfg, os));
        CHECK_OP(os, urls);
        addObjects(d.data(), context, dataCfg, data, os);
        CHECK_OP(os, urls);

        DocumentFormat *f = getFormat(dataCfg, os);
        CHECK_OP(os, urls);
        f->storeDocument(d.data(), os);
        CHECK_OP(os, urls);
        urls << d->getURLString();
        paramValue = "\"" + d->getURLString() + "\"";
    }

    execString.replace(ind, dataCfg.attrName.size() + 1 , paramValue);
    return urls;
}

QString ExternalProcessWorker::prepareOutput(QString &execString, const DataConfig &dataCfg, U2OpStatus &os) {
    int ind = execString.indexOf(QRegExp("\\$" + dataCfg.attrName + "(\\W|$)"));
    CHECK(-1 != ind, "");

    QString extension;
    if (dataCfg.isFileUrl()) {
        extension = "tmp";
    } else {
        DocumentFormat *f = getFormat(dataCfg, os);
        CHECK_OP(os, "");
        extension = f->getSupportedDocumentFileExtensions().first();
    }
    QString url = generateAndCreateURL(extension, dataCfg.attrName);
    execString.replace(ind, dataCfg.attrName.size() + 1 , "\"" + url + "\"");

    return url;
}

Task * ExternalProcessWorker::tick() {
    CHECK(!finishWorkIfInputEnded(), NULL);

    QString execString = commandLine;
    applyAttributes(execString);

    int i = 0;
    foreach(const DataConfig &dataCfg, cfg->inputs) { //write all input data to files
        Message inputMessage = getMessageAndSetupScriptValues(inputs[i]);
        i++;
        QVariantMap data = inputMessage.getData().toMap();
        U2OpStatusImpl os;
        inputUrls << applyInputMessage(execString, dataCfg, data, os);
        CHECK_OP(os, new FailTask(os.getError()));
    }

    QMap<QString, DataConfig> outputUrls;
    foreach(const DataConfig &dataCfg, cfg->outputs) {
        U2OpStatusImpl os;
        QString url = prepareOutput(execString, dataCfg, os);
        CHECK_OP(os, new FailTask(os.getError()));
        if (!url.isEmpty()) {
            outputUrls[url] = dataCfg;
        }
    }

    LaunchExternalToolTask *task = new LaunchExternalToolTask(execString, outputUrls);
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_onTaskFinishied()));
    return task;
}

bool ExternalProcessWorker::finishWorkIfInputEnded() {
    bool hasMessages = true;
    bool isEnded = true;
    checkInputBusState(hasMessages, isEnded);
    if (!hasMessages && isEnded) {
        if (NULL != output) {
            output->setEnded();
        }
        setDone();
        return true;
    } else {
        return false;
    }
}

namespace {
static GObject * getObject(Document *d, GObjectType t, U2OpStatus &os) {
    QList<GObject*> objs = d->findGObjectByType(t, UOF_LoadedAndUnloaded);
    if (objs.isEmpty()) {
        os.setError(QObject::tr("No target objects in the file: %1").arg(d->getURLString()));
        return NULL;
    }
    return objs.first();
}

static SharedDbiDataHandler getAlignment(Document *d, WorkflowContext *context, U2OpStatus &os) {
    GObject *obj = getObject(d, GObjectTypes::MULTIPLE_ALIGNMENT, os);
    CHECK_OP(os, SharedDbiDataHandler());

    MAlignmentObject *msaObj =  static_cast<MAlignmentObject*>(obj);
    if (NULL == msaObj) {
        os.setError(QObject::tr("Error with alignment object"));
        return SharedDbiDataHandler();
    }
    return context->getDataStorage()->getDataHandler(msaObj->getEntityRef());
}

static SharedDbiDataHandler getAnnotations(Document *d, WorkflowContext *context, U2OpStatus &os)
{
    GObject *obj = getObject(d, GObjectTypes::ANNOTATION_TABLE, os);
    CHECK_OP(os, SharedDbiDataHandler());

    AnnotationTableObject *annsObj = static_cast<AnnotationTableObject *>(obj);
    if (NULL == annsObj) {
        os.setError(QObject::tr("Error with annotations object"));
        return SharedDbiDataHandler();
    }
    return context->getDataStorage()->getDataHandler(annsObj->getEntityRef());
}

} // namespace

void ExternalProcessWorker::sl_onTaskFinishied() {
    LaunchExternalToolTask *t = static_cast<LaunchExternalToolTask*>(sender());
    CHECK(output && t->isFinished() && !t->hasError(),);

    /* This variable and corresponded code parts with it
     * are temporary created for merging sequences.
     * When standard multiplexing/merging tools will be created
     * then the variable and code parts must be deleted.
     */
    QMap<QString, QList<U2EntityRef> > seqsForMergingBySlotId;
    QMap<QString, DataConfig> outputUrls = t->takeOutputUrls();
    QMap<QString, DataConfig>::iterator i = outputUrls.begin();
    QVariantMap v;
    for(; i != outputUrls.end(); i++) {
        DataConfig cfg = i.value();
        QString url = i.key();

        if (cfg.isFileUrl()) {
            if (QFile::exists(url)) {
                DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                v[WorkflowUtils::getSlotDescOfDatatype(dataType).getId()] = url;
                context->addExternalProcessFile(url);
            } else {
                reportError(tr("%1 file was not created").arg(url));
            }
        } else {
            U2OpStatusImpl os;
            QScopedPointer<Document> d(loadDocument(url, cfg, context, os));
            CHECK_OP_EXT(os, reportError(os.getError()),);
            d->setDocumentOwnsDbiResources(false);

            if (cfg.isSequence()){
                QList<GObject*> seqObjects = d->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded);
                DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                QString slotId = WorkflowUtils::getSlotDescOfDatatype(dataType).getId();
                if (1 == seqObjects.size()) {
                    GObject *obj = seqObjects.first();
                    Workflow::SharedDbiDataHandler id = context->getDataStorage()->getDataHandler(obj->getEntityRef());
                    v[slotId] = qVariantFromValue<SharedDbiDataHandler>(id);
                } else if (1 < seqObjects.size()) {
                    QList<U2EntityRef> refs;
                    foreach (GObject *obj, seqObjects) {
                        refs << obj->getEntityRef();
                    }
                    seqsForMergingBySlotId.insert(slotId, refs);
                }
            } else if (cfg.isAlignment()) {
                SharedDbiDataHandler msaId = getAlignment(d.data(), context, os);
                CHECK_OP_EXT(os, reportError(os.getError()),);
                DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                v[WorkflowUtils::getSlotDescOfDatatype(dataType).getId()] = qVariantFromValue<SharedDbiDataHandler>(msaId);
            } else if (cfg.isAnnotations()) {
                const SharedDbiDataHandler annTableId = getAnnotations(d.data(), context, os);
                CHECK_OP_EXT(os, reportError(os.getError()),);
                DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                v[WorkflowUtils::getSlotDescOfDatatype(dataType).getId()] = qVariantFromValue<SharedDbiDataHandler>(annTableId);
            } else if (cfg.isAnnotatedSequence()) {
                if(!d->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded).isEmpty()) {
                    U2SequenceObject *seqObj = static_cast<U2SequenceObject *>(d->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded).first());
                    DNASequence seq = seqObj->getWholeSequence(os);
                    CHECK_OP_EXT(os, reportError(os.getError()),);
                    seq.alphabet = U2AlphabetUtils::getById(BaseDNAAlphabetIds::RAW());
                    SharedDbiDataHandler seqId = context->getDataStorage()->putSequence(seq);
                    v[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(seqId);
                }
                U2OpStatusImpl os;

                const SharedDbiDataHandler annTableId = getAnnotations(d.data(), context, os);
                if (!os.hasError()) {
                    DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                    v[BaseSlots::ANNOTATION_TABLE_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(annTableId);
                }
            } else if (cfg.isText()) {
                if(!d->findGObjectByType(GObjectTypes::TEXT, UOF_LoadedAndUnloaded).isEmpty()) {
                    TextObject *obj = static_cast<TextObject*>(d->findGObjectByType(GObjectTypes::TEXT, UOF_LoadedAndUnloaded).first());
                    DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                    v[WorkflowUtils::getSlotDescOfDatatype(dataType).getId()] = qVariantFromValue<QString>(obj->getText());
                }
            }

            QFile::remove(url);
        }
    }

    DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(OUTPUT_PORT_TYPE + cfg->name);

    if (seqsForMergingBySlotId.isEmpty()) {
        output->put(Message(dataType, v));
    } else if (1 == seqsForMergingBySlotId.size()) {
        // create a message for every sequence
        QString slotId = seqsForMergingBySlotId.keys().first();
        const QList<U2EntityRef> &refs= seqsForMergingBySlotId.value(slotId);
        foreach(const U2EntityRef &eRef, refs) {
            SharedDbiDataHandler id = context->getDataStorage()->getDataHandler(eRef);
            v[slotId] = qVariantFromValue<SharedDbiDataHandler>(id);
            output->put(Message(dataType, v));
        }
    } else {
        // merge every sequence group and send one message
        U2SequenceImporter seqImporter = U2SequenceImporter(QVariantMap());
        U2OpStatus2Log os;

        foreach (const QString &slotId, seqsForMergingBySlotId.keys()) {
            const QList<U2EntityRef> &refs= seqsForMergingBySlotId.value(slotId);
            bool first = true;
            foreach(const U2EntityRef &eRef, refs) {
                QScopedPointer<U2SequenceObject> obj(new U2SequenceObject("tmp_name", eRef));
                if (first) {
                    seqImporter.startSequence(context->getDataStorage()->getDbiRef(), U2ObjectDbi::ROOT_FOLDER, slotId, false, os);
                    first = false;
                }
                U2Region wholeSeq(0, obj->getSequenceLength());
                seqImporter.addSequenceBlock(eRef, wholeSeq, os);
            }
            U2Sequence seq = seqImporter.finalizeSequenceAndValidate(os);
            U2EntityRef eRef(context->getDataStorage()->getDbiRef(), seq.id);
            SharedDbiDataHandler id = context->getDataStorage()->getDataHandler(eRef);
            v[slotId] = qVariantFromValue<SharedDbiDataHandler>(id);
        }
        CHECK_OP(os,);
        output->put(Message(dataType, v));
    }
}

void ExternalProcessWorker::init() {
    output = ports.value(OUT_PORT_ID);

    foreach(const DataConfig& input, cfg->inputs) {
        IntegralBus *inBus = ports.value(input.attrName);
        inputs << inBus;

        inBus->addComplement(output);
    }
}

void ExternalProcessWorker::checkInputBusState(bool &hasMessages, bool &isEnded) const {
    hasMessages = true;
    isEnded = false;
    foreach(const CommunicationChannel *ch, inputs) {
        if (NULL != ch) {
            hasMessages &= static_cast<bool>(ch->hasMessage());
            isEnded |= ch->isEnded();
        }
    }
}

bool ExternalProcessWorker::isReady() const {
    CHECK(!isDone(), false);
    if(inputs.isEmpty()) {
        return true;
    } else {
        bool hasMessages = true;
        bool isEnded = true;
        checkInputBusState(hasMessages, isEnded);
        return hasMessages || isEnded;
    }
}

void ExternalProcessWorker::cleanup() {
    foreach(const QString& url, inputUrls) {
        if(QFile::exists(url)) {
            QFile::remove(url);
        }
    }
}

/************************************************************************/
/* LaunchExternalToolTask */
/************************************************************************/
LaunchExternalToolTask::LaunchExternalToolTask(const QString &execString, const QMap<QString, DataConfig> &outputUrls)
: Task(tr("Launch external process task"), TaskFlag_None), outputUrls(outputUrls), execString(execString)
{

}

LaunchExternalToolTask::~LaunchExternalToolTask() {
    foreach(const QString &url, outputUrls.keys()) {
        if (QFile::exists(url)) {
            QFile::remove(url);
        }
    }
}

// a function from "qprocess.cpp"
QStringList LaunchExternalToolTask::parseCombinedArgString(const QString &program)
{
    QStringList args;
    QString tmp;
    int quoteCount = 0;
    bool inQuote = false;

    // handle quoting. tokens can be surrounded by double quotes
    // "hello world". three consecutive double quotes represent
    // the quote character itself.
    for (int i = 0; i < program.size(); ++i) {
        if (program.at(i) == QLatin1Char('"')) {
            ++quoteCount;
            if (quoteCount == 3) {
                // third consecutive quote
                quoteCount = 0;
                tmp += program.at(i);
            }
            continue;
        }
        if (quoteCount) {
            if (quoteCount == 1)
                inQuote = !inQuote;
            quoteCount = 0;
        }
        if (!inQuote && program.at(i).isSpace()) {
            if (!tmp.isEmpty()) {
                args += tmp;
                tmp.clear();
            }
        } else {
            tmp += program.at(i);
        }
    }
    if (!tmp.isEmpty())
        args += tmp;

    return args;
}

#define WIN_LAUNCH_CMD_COMMAND "cmd /C "
#define START_WAIT_MSEC 3000

void LaunchExternalToolTask::run() {
    QProcess *externalProcess = new QProcess();
    if(execString.contains(">")) {
        QString output = execString.split(">").last();
        output = output.trimmed();
        if(output.at(0) == '\"') {
            output = output.mid(1, output.length() - 2);
        }
        execString = execString.split(">").first();
        externalProcess->setStandardOutputFile(output);
    }

    QStringList execStringArgs = parseCombinedArgString(execString);
    QString execStringProg = execStringArgs.takeAt(0);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    externalProcess->setProcessEnvironment(env);
    taskLog.details(tr("Running external process: %1").arg(execString));

    bool startOk = WorkflowUtils::startExternalProcess(externalProcess, execStringProg, execStringArgs);

    if(!startOk) {
        stateInfo.setError(tr("Can't launch %1").arg(execString));
        return;
    }

    while(!externalProcess->waitForFinished(1000)) {
        if(isCanceled()) {
            externalProcess->kill();
        }
    }
}

QMap<QString, DataConfig> LaunchExternalToolTask::takeOutputUrls() {
    QMap<QString, DataConfig> result = outputUrls;
    outputUrls.clear();
    return result;
}

/************************************************************************/
/* ExternalProcessWorkerPrompter */
/************************************************************************/
QString ExternalProcessWorkerPrompter::composeRichDoc() {
    ExternalProcessConfig *cfg = WorkflowEnv::getExternalCfgRegistry()->getConfigByName(target->getProto()->getId());
    assert(cfg);
    QString doc = cfg->templateDescription;

    foreach(const DataConfig& dataCfg, cfg->inputs) {
        QRegExp param("\\$" + dataCfg.attrName + /*"[,:;\s\.\-]"*/"\\W|$");
        if(doc.contains(param)) {
            IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(dataCfg.attrName));
            DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(dataCfg.type);
            if(dataCfg.type == SEQ_WITH_ANNS) {
                dataType = BaseTypes::DNA_SEQUENCE_TYPE();
            }
            Actor* producer = input->getProducer(WorkflowUtils::getSlotDescOfDatatype(dataType).getId());
            QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
            QString producerName = tr("<u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);
            doc.replace("$" + dataCfg.attrName, producerName);
        }
    }

    foreach(const DataConfig& dataCfg, cfg->outputs) {
        QRegExp param("\\$" + dataCfg.attrName + /*"[,:;\s\.\-]"*/"\\W|$");
        if(doc.contains(param)) {
            IntegralBusPort* output = qobject_cast<IntegralBusPort*>(target->getPort(OUT_PORT_ID));
            DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(dataCfg.type);
            if(dataCfg.type == SEQ_WITH_ANNS) {
                dataType = BaseTypes::DNA_SEQUENCE_TYPE();
            }
            QString destinations;
            QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
            if(!output->getLinks().isEmpty()) {
                foreach(Port *p, output->getLinks().keys()) {
                    IntegralBusPort* ibp = qobject_cast<IntegralBusPort*>(p);
                    Actor *dest = ibp->owner();
                    destinations += tr("<u>%1</u>").arg(dest ? dest->getLabel() : unsetStr) + ",";
                }
            }
            if(destinations.isEmpty()) {
                destinations = tr("<u>%1</u>").arg(unsetStr);
            } else {
                destinations.resize(destinations.size() - 1); //remove last semicolon
            }
            doc.replace("$" + dataCfg.attrName, destinations);
        }
    }

    foreach(const AttributeConfig &attrCfg, cfg->attrs) {
        QRegExp param("\\$" + attrCfg.attrName + /*"[,:;\s\.\-]"*/"\\W|$");
        if(doc.contains(param)) {
            QString prm = getRequiredParam(attrCfg.attrName);
            doc.replace("$" + attrCfg.attrName, getHyperlink(attrCfg.attrName, prm));
        }
    }

    return doc;
}


}
}
