/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
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
#include <memory>

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

const QString ExternalProcessWorker::generateAndCreateURL(const QString &extention, const QString &name) {
    QString url;
    QString path = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath("wd_external");
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(path);
    }
    url = path + "/tmp" + name + QString::number(QDateTime::currentDateTime().toTime_t()) +  "." + extention;
    return url;
}

class BusyHelper {
public:
    BusyHelper(bool& _b) : b(_b), keepBusy(false){ b = true;}
    ~BusyHelper() {b = keepBusy;}
    bool& b;
    bool keepBusy;
};

Task* ExternalProcessWorker::tick() {
    CHECK(!busy, NULL);
    
    BusyHelper busyLock(busy);

    QString execString = commandLine;
    foreach(Attribute *a, actor->getAttributes()) {
        int i = execString.indexOf(QRegExp("\\$" + a->getDisplayName() + "(\\W|$)"));
        if(i != -1) {
            execString.replace(i, a->getDisplayName().size() + 1 , a->getAttributeValue<QString>(context)); //set parameters in command line with attributes values
        }
    }

    int i = 0;
    U2OpStatus2Log os;
    foreach(const DataConfig& dataCfg, cfg->inputs) { //write all input data to files
        Message inputMessage = getMessageAndSetupScriptValues(inputs[i]);
        QVariantMap qm = inputMessage.getData().toMap();
        QString paramValue;

        if (BaseTypes::STRING_TYPE()->getId() == dataCfg.type
            && DataConfig::StringValue == dataCfg.format) {
            paramValue = qm.value(BaseSlots::TEXT_SLOT().getId()).value<QString>();
        } else {
            DocumentFormat *f = AppContext::getDocumentFormatRegistry()->getFormatById(dataCfg.format);
            IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            QString url = generateAndCreateURL(f->getSupportedDocumentFileExtensions().first(), dataCfg.attrName);
            inputUrls << url;
            std::auto_ptr<Document> d(f->createNewLoadedDocument(iof, url, os));
            CHECK_OP(os, NULL);

            if (dataCfg.type == BaseTypes::DNA_SEQUENCE_TYPE()->getId()) {
                U2DataId seqId = qm.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<U2DataId>();
                U2SequenceObject *seqObj = StorageUtils::getSequenceObject(context->getDataStorage(), seqId);
                if (NULL == seqObj) {
                    return NULL;
                }
                d->addObject(seqObj);
            } else if(dataCfg.type == BaseTypes::ANNOTATION_TABLE_TYPE()->getId()) {
                QList<SharedAnnotationData>  anns = qm.value(BaseSlots::ANNOTATION_TABLE_SLOT().getId()).value<QList<SharedAnnotationData> >();
                AnnotationTableObject * aobj = new AnnotationTableObject("anns");
                foreach(const SharedAnnotationData& ann, anns) {
                    QStringList list;
                    aobj->addAnnotation(new Annotation(ann));
                }
                d->addObject(aobj);
            } else if(dataCfg.type == BaseTypes::MULTIPLE_ALIGNMENT_TYPE()->getId()) {
                MAlignment ma = qm.value(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()).value<MAlignment>();
                d->addObject(new MAlignmentObject(ma));
            } else if (dataCfg.type == SEQ_WITH_ANNS) {
                U2DataId seqId = qm.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<U2DataId>();
                U2SequenceObject *dnaObj = StorageUtils::getSequenceObject(context->getDataStorage(), seqId);
                if (NULL == dnaObj) {
                    return NULL;
                }
                d->addObject(dnaObj);
                
                QList<SharedAnnotationData>  anns = qm.value(BaseSlots::ANNOTATION_TABLE_SLOT().getId()).value<QList<SharedAnnotationData> >();
                AnnotationTableObject * aobj = new AnnotationTableObject("anns");
                foreach(const SharedAnnotationData& ann, anns) {
                    aobj->addAnnotation(new Annotation(ann));
                }
                
                d->addObject(aobj);
                
                QList<GObjectRelation> rel;
                rel << GObjectRelation(GObjectReference(dnaObj), GObjectRelationRole::SEQUENCE);
                aobj->setObjectRelations(rel);
            } else if(dataCfg.type == BaseTypes::STRING_TYPE()->getId()) {
                QString str = qm.value(BaseSlots::TEXT_SLOT().getId()).value<QString>();
                d->addObject(new TextObject(str, "tmp_text_object"));
            }

            f->storeDocument(d.get(), os);
            CHECK_OP(os, NULL);
            paramValue = url;
        }

        int ind = execString.indexOf(QRegExp("\\$" + dataCfg.attrName + "(\\W|$)"));
        if (ind != -1) {
            execString.replace(ind, dataCfg.attrName.size() + 1 , paramValue); 
        }
    }

    foreach(const DataConfig &dataCfg, cfg->outputs) { 
        QString url1 = generateAndCreateURL(AppContext::getDocumentFormatRegistry()->getFormatById(dataCfg.format)->getSupportedDocumentFileExtensions().first(), dataCfg.attrName);
        outputUrls.insert(url1, dataCfg);

        int ind = execString.indexOf(QRegExp("\\$" + dataCfg.attrName + "(\\W|$)"));
        if(ind != -1) {
            execString.replace(ind, dataCfg.attrName.size() + 1 , "\"" + url1 + "\""); 
        }
    }
    
    LaunchExternalToolTask *task = new LaunchExternalToolTask(execString);
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_onTaskFinishied()));
    busyLock.keepBusy = true;
    return task;
}

void ExternalProcessWorker::sl_onTaskFinishied() {
    Task *t = static_cast<Task*>(sender());
    if(output && t->isFinished() && !t->hasError()) {
        QMap<QString, DataConfig>::iterator i;
        QVariantMap v;

        /* This variable and corresponded code parts with it
         * are temporary created for merging sequences.
         * When standard multiplexing/merging tools will be created
         * then the variable and code parts must be deleted.
         */
        QMap<QString, QList<U2EntityRef> > seqsForMergingBySlotId;

        for(i = outputUrls.begin(); i != outputUrls.end(); i++) {
            DataConfig cfg = i.value();
            QString url = i.key();

            DocumentFormat *f = AppContext::getDocumentFormatRegistry()->getFormatById(cfg.format);
            IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            U2OpStatus2Log os;
            QVariantMap hints;
            hints.insert(DocumentFormat::DBI_ALIAS_HINT, QString(WORKFLOW_SESSION_TMP_DBI_ALIAS));
            std::auto_ptr<Document> d(f->loadDocument(iof, url, hints, os));
            d->setDocumentOwnsDbiResources(false);
            
            if (os.hasError()) {
                //coreLog.error(tr("Can't open document"));
                bool isEnded = true;
                foreach(CommunicationChannel *ch, inputs) {
                    isEnded = isEnded && ch->isEnded();
                }
                if(isEnded) {
                    output->setEnded();
                    done  = true;
                }
                busy = false;
                QFile::remove(url);
                outputUrls.clear();
                return;
            }

            if( cfg.type == BaseTypes::DNA_SEQUENCE_TYPE()->getId()){
                QList<GObject*> seqObjects = d->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded);
                DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                QString slotId = WorkflowUtils::getSlotDescOfDatatype(dataType).getId();
                if (1 == seqObjects.size()) {
                    GObject *obj = seqObjects.first();
                    v[slotId] = obj->getEntityRef().entityId;
                } else if (1 < seqObjects.size()) {
                    QList<U2EntityRef> refs;
                    foreach (GObject *obj, seqObjects) {
                        refs << obj->getEntityRef();
                    }
                    seqsForMergingBySlotId.insert(slotId, refs);
                }
            } else if(cfg.type == BaseTypes::MULTIPLE_ALIGNMENT_TYPE()->getId()) {
                MAlignmentObject *obj =  static_cast<MAlignmentObject *> (d->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT, UOF_LoadedAndUnloaded).first());
                DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                v[WorkflowUtils::getSlotDescOfDatatype(dataType).getId()] = qVariantFromValue<MAlignment>(obj->getMAlignment());
            } else if(cfg.type == BaseTypes::ANNOTATION_TABLE_TYPE()->getId()) {
                AnnotationTableObject *obj = static_cast<AnnotationTableObject *>(d->findGObjectByType(GObjectTypes::ANNOTATION_TABLE, UOF_LoadedAndUnloaded).first());
                QList<SharedAnnotationData> list;
                foreach(Annotation* a, obj->getAnnotations()) {
                    list << a->data();
                }
                DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                v[WorkflowUtils::getSlotDescOfDatatype(dataType).getId()] = qVariantFromValue(list);
            } else if(cfg.type == SEQ_WITH_ANNS) {
                if(!d->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded).isEmpty()) {
                    U2SequenceObject *seqObj = static_cast<U2SequenceObject *>(d->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded).first());
                    DNASequence seq = seqObj->getWholeSequence();
                    seq.alphabet = U2AlphabetUtils::getById(BaseDNAAlphabetIds::RAW());
                    U2DataId seqId = context->getDataStorage()->putSequence(seq);
                    v[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = seqId;
                }
                if(!d->findGObjectByType(GObjectTypes::ANNOTATION_TABLE, UOF_LoadedAndUnloaded).isEmpty()) {
                    AnnotationTableObject *obj = static_cast<AnnotationTableObject *>(d->findGObjectByType(GObjectTypes::ANNOTATION_TABLE, UOF_LoadedAndUnloaded).first());
                    QList<SharedAnnotationData> list;
                    foreach(Annotation* a, obj->getAnnotations()) {
                        list << a->data();
                    }
                    v[BaseSlots::ANNOTATION_TABLE_SLOT().getId()] = qVariantFromValue(list);
                }
                
            } else if(cfg.type == BaseTypes::STRING_TYPE()->getId()) {
                if(!d->findGObjectByType(GObjectTypes::TEXT, UOF_LoadedAndUnloaded).isEmpty()) {
                    TextObject *obj = static_cast<TextObject*>(d->findGObjectByType(GObjectTypes::TEXT, UOF_LoadedAndUnloaded).first());
                    DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                    v[WorkflowUtils::getSlotDescOfDatatype(dataType).getId()] = qVariantFromValue<QString>(obj->getText());
                }
            }
            QFile::remove(url);
        }

        outputUrls.clear();
        DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(OUTPUT_PORT_TYPE + cfg->name);
        
        if (seqsForMergingBySlotId.isEmpty()) {
            output->put(Message(dataType, v));
        } else if (1 == seqsForMergingBySlotId.size()) {
            // create a message for every sequence
            QString slotId = seqsForMergingBySlotId.keys().first();
            const QList<U2EntityRef> &refs= seqsForMergingBySlotId.value(slotId);
            foreach(const U2EntityRef &eRef, refs) {
                v[slotId] = eRef.entityId;
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
                    std::auto_ptr<U2SequenceObject> obj(new U2SequenceObject("tmp_name", eRef));
                    if (first) {
                        seqImporter.startSequence(eRef.dbiRef, slotId, false, os);
                        first = false;
                    }
                    U2Region wholeSeq(0, obj->getSequenceLength());
                    seqImporter.addSequenceBlock(eRef, wholeSeq, os);
                }
                U2Sequence seq = seqImporter.finalizeSequence(os);
                v[slotId] = seq.id;
            }
            output->put(Message(dataType, v));
        }
        bool isEnded = true;
        foreach(CommunicationChannel *ch, inputs) {
            isEnded = isEnded && ch->isEnded();
        }
        if(isEnded) {
            output->setEnded();
            done = true;
        }
    } else {
        bool res = true;
        foreach(const CommunicationChannel *ch, inputs) {
            if(ch) {
                res = res && ch->hasMessage();
            }
        }
        done = !res;
    }
    busy = false;
}

void ExternalProcessWorker::init() {
    output = ports.value(OUT_PORT_ID);

    foreach(const DataConfig& input, cfg->inputs) {
        IntegralBus *inBus = ports.value(input.attrName);
        inputs << inBus;

        inBus->addComplement(output);
    }
}

bool ExternalProcessWorker::isReady() {
    if(inputs.isEmpty()) {
        return !done;
    } else {
        bool res = true;
        foreach(const CommunicationChannel *ch, inputs) {
            if(ch) {
                res = res && ch->hasMessage();
            }
        }
        return res;
    }
}

bool ExternalProcessWorker::isDone() {
    /*bool res = true;
    foreach(CommunicationChannel *ch, inputs) {
        res = res && ch->isEnded();
    }*/

    return done;
}

void ExternalProcessWorker::cleanup() {
    foreach(const QString& url, inputUrls) {
        if(QFile::exists(url)) {
            QFile::remove(url);
        }
    }

    foreach(const QString& url, outputUrls.keys()) {
        if(QFile::exists(url)) {
            QFile::remove(url);
        }
    }
}

// a function from "qprocess.cpp"
static QStringList parseCombinedArgString(const QString &program)
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

LaunchExternalToolTask::LaunchExternalToolTask(const QString &_execString):
Task("Launch external process task", TaskFlag_None), execString(_execString)  {

}

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
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    externalProcess->setProcessEnvironment(env);
    externalProcess->start(execString);
    taskLog.details(tr("Running external process: %1").arg(execString));

    bool startOk = externalProcess->waitForStarted(START_WAIT_MSEC);

#ifdef Q_OS_WIN32
    if(!startOk) {
        QStringList args = parseCombinedArgString(execString);
        if (!args.isEmpty()) {
            QFileInfo fi(args.first());
            if (!fi.isAbsolute()) {
                QString append = args.size() > 1 ? " ..." : "";
                taskLog.error(tr("Can't run an executable file \"%1%2\". Try to run it as a cmd line command: \"%3%4\"")
                    .arg(args.first()).arg(append).arg(WIN_LAUNCH_CMD_COMMAND + args.first()).arg(append));
                externalProcess->start(WIN_LAUNCH_CMD_COMMAND + execString);
                taskLog.details(tr("Running external process: %1").arg(WIN_LAUNCH_CMD_COMMAND + execString));
                startOk = externalProcess->waitForStarted(START_WAIT_MSEC);
            }
        }
    }
#endif

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
