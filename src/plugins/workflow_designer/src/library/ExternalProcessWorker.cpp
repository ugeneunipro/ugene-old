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

#include "ExternalProcessWorker.h"
#include <U2Lang/BaseTypes.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/ExternalToolCfg.h>
#include <U2Core/AppSettings.h>
#include <QtCore/QDateTime>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Lang/BaseSlots.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GObjectRelationRoles.h>


namespace U2 {
namespace LocalWorkflow {

const static QString INPUT_PORT_TYPE("input-for-");
const static QString OUTPUT_PORT_TYPE("output-for-");
static const QString OUT_PORT_ID("out");

bool ExternalProcessWorkerFactory::init(ExternalProcessConfig *cfg) {
    DataTypeRegistry *dtr = WorkflowEnv::getDataTypeRegistry();
    QList<PortDescriptor*> portDescs; 
    foreach(const DataConfig& dcfg, cfg->inputs) {
        QMap<Descriptor, DataTypePtr> map;
        if(dcfg.type == SEQ_WITH_ANNS) {
            map[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
            map[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
        } else {
            map[WorkflowUtils::getSlotDescOfDatatype(dtr->getById(dcfg.type))] = dtr->getById(dcfg.type);
        }

        DataTypePtr input( new MapDataType(Descriptor(INPUT_PORT_TYPE + dcfg.attrName), map) );
        DataTypeRegistry * dr = WorkflowEnv::getDataTypeRegistry();
        assert(dr);
        dr->registerEntry( input );
        portDescs << new PortDescriptor(Descriptor(dcfg.attrName, dcfg.attrName, dcfg.description), input, true);
    }

    QMap<Descriptor, DataTypePtr> map;
    foreach(const DataConfig& dcfg, cfg->outputs) {
        if(dcfg.type == SEQ_WITH_ANNS) {
            map[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
            map[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        } else {
            map[WorkflowUtils::getSlotDescOfDatatype(dtr->getById(dcfg.type))] = dtr->getById(dcfg.type);
        }
    }
    if(!map.isEmpty()) {
        DataTypePtr outSet( new MapDataType(Descriptor(OUTPUT_PORT_TYPE + cfg->name), map) );
        DataTypeRegistry * dr = WorkflowEnv::getDataTypeRegistry();
        assert(dr);
        dr->registerEntry( outSet );
        Descriptor outDesc( OUT_PORT_ID, ExternalProcessWorker::tr("output data"), ExternalProcessWorker::tr("output data") );
        portDescs << new PortDescriptor( outDesc, outSet, false, true );
    }

    Descriptor desc( cfg->name, cfg->name, cfg->description.isEmpty() ? cfg->name : cfg->description );

    QList<Attribute*> attribs;
    QMap<QString, PropertyDelegate*> delegates;
    foreach(const AttributeConfig& acfg, cfg->attrs) {
        //PropertyDelegate *delegate = NULL;
        DataTypePtr type;
        QString descr = acfg.description.isEmpty() ? acfg.type : acfg.description;
        if(acfg.type == "URL") {
            type = BaseTypes::STRING_TYPE();
            delegates[acfg.attrName] = new URLDelegate("All Files(*.*)","");
            attribs << new Attribute(Descriptor(acfg.attrName, acfg.attrName, descr), type);
        } else if(acfg.type == "String") {
            type = BaseTypes::STRING_TYPE();
            attribs << new Attribute(Descriptor(acfg.attrName, acfg.attrName, descr), type);
        } else if(acfg.type == "Number") {
            type = BaseTypes::NUM_TYPE();
            attribs << new Attribute(Descriptor(acfg.attrName, acfg.attrName, descr), type);
        } else if(acfg.type == "Boolean") {
            type = BaseTypes::BOOL_TYPE();
            attribs << new Attribute(Descriptor(acfg.attrName, acfg.attrName, descr), type, false, QVariant(false));
        }

        //attribs << new Attribute(Descriptor(acfg.attrName, acfg.attrName, acfg.type), type);
        /*if(delegate) {
            delegates[acfg.attrName] = acfg.delegate;
        }*/
    }

    ActorPrototype * proto = new IntegralBusActorPrototype( desc, portDescs, attribs );

    proto->setEditor(new DelegateEditor(delegates));
    proto->setIconPath(":workflow_designer/images/external_cmd_tool.png");

    proto->setPrompter( new ExternalProcessWorkerPrompter() );
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_EXTERNAL(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById( LocalDomainFactory::ID );
    WorkflowEnv::getExternalCfgRegistry()->registerExternalTool(cfg);
    localDomain->registerEntry( new ExternalProcessWorkerFactory(cfg->name) );
    return true;
}

const QString ExternalProcessWorker::generateURL(const QString &extention, const QString &name) {
    QString url;
    QString path = AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath();
    url = path + "/tmp" + name + QString::number(QDateTime::currentDateTime().toTime_t()) +  "." + extention;
    return url;
}

Task* ExternalProcessWorker::tick() {
    if(busy) {
        return NULL;
    }
    busy = true;
    QString execString = commandLine;
    foreach(Attribute *a, actor->getAttributes()) {
        int i = execString.indexOf(QRegExp("\\$" + a->getDisplayName() + "(\\W|$)"));
        if(i != -1) {
            //commandLine.replace("$" + a->getDisplayName(), a->getAttributeValue<QString>()); //set parameters in command line with attributes values
            execString.replace(i, a->getDisplayName().size() + 1 , a->getAttributeValue<QString>()); //set parameters in command line with attributes values
        }
    }

    int i = 0;
    
    foreach(const DataConfig& dataCfg, cfg->inputs) { //write all input data to files
        DocumentFormat *f;
        f = AppContext::getDocumentFormatRegistry()->getFormatById(dataCfg.format);
        IOAdapterFactory *io = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        QString url = generateURL(f->getSupportedDocumentFileExtensions().first(), dataCfg.attrName);
        inputUrls << url;
        /*Document *d = new Document(f, io, url);
        d->setLoaded(true);*/
        Document *d;


        Message inputMessage = getMessageAndSetupScriptValues(inputs[i]);
        QList<GObject*> l;
        QVariantMap qm = inputMessage.getData().toMap();
        if(dataCfg.type == BaseTypes::DNA_SEQUENCE_TYPE()->getId()) {
            DNASequence seq = qm.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<DNASequence>();
            //d->addObject(new DNASequenceObject(seq.getName(), seq));
            l << new DNASequenceObject(seq.getName(), seq);
            d = new Document(f, io, url, l);
        } else if(dataCfg.type == BaseTypes::ANNOTATION_TABLE_TYPE()->getId()) {
            QList<SharedAnnotationData>  anns = qm.value(BaseSlots::ANNOTATION_TABLE_SLOT().getId()).value<QList<SharedAnnotationData> >();
            AnnotationTableObject * aobj = new AnnotationTableObject("anns");
            foreach(SharedAnnotationData ann, anns) {
                QStringList list;
                aobj->addAnnotation(new Annotation(ann));
            }
            l << aobj;
            //d->addObject(aobj);
            d = new Document(f, io, url, l);
        } else if(dataCfg.type == BaseTypes::MULTIPLE_ALIGNMENT_TYPE()->getId()) {
            MAlignment ma = qm.value(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()).value<MAlignment>();
            l << new MAlignmentObject(ma);
            //d->addObject(new MAlignmentObject(ma));
            d = new Document(f, io, url, l);
        } else if(dataCfg.type == SEQ_WITH_ANNS) {
            DNASequence seq = qm.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<DNASequence>();
            DNASequenceObject*dnaObj =  new DNASequenceObject(seq.getName(), seq);
            //d->addObject(dnaObj);
            l << dnaObj;

            QList<SharedAnnotationData>  anns = qm.value(BaseSlots::ANNOTATION_TABLE_SLOT().getId()).value<QList<SharedAnnotationData> >();
            AnnotationTableObject * aobj = new AnnotationTableObject("anns");
            foreach(SharedAnnotationData ann, anns) {
                QStringList list;
                aobj->addAnnotation(new Annotation(ann));
            }
            
            l << aobj;
            d = new Document(f, io, url, l);

            QList<GObjectRelation> rel;
            rel << GObjectRelation(GObjectReference(dnaObj), GObjectRelationRole::SEQUENCE);
            aobj->setObjectRelations(rel);
        }
        //IOAdapterFactory *io = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        //QString url = generateURL(f->getSupportedDocumentFileExtensions().first(), dataCfg.attrName);
        //inputUrls << url;
        //Document *d = new Document(f, io, url, l);
        TaskStateInfo ts;
        f->storeDocument(d, ts, io);

        int ind = execString.indexOf(QRegExp("\\$" + dataCfg.attrName + "(\\W|$)"));
        if(ind != -1) {
            execString.replace(ind, dataCfg.attrName.size() + 1 , url); 
        }
        //commandLine.replace("$" + dataCfg.attrName, url);
        
        delete d;
        i++;
    }

    foreach(const DataConfig &dataCfg, cfg->outputs) { 
        QString url1 = generateURL(AppContext::getDocumentFormatRegistry()->getFormatById(dataCfg.format)->getSupportedDocumentFileExtensions().first(), dataCfg.attrName);
        outputUrls.insert(url1, dataCfg);

        int ind = execString.indexOf(QRegExp("\\$" + dataCfg.attrName + "(\\W|$)"));
        if(ind != -1) {
            execString.replace(ind, dataCfg.attrName.size() + 1 , "\"" + url1 + "\""); 
        }
        //commandLine.replace("$" + dataCfg.attrName, url1);
    }
    
    LaunchExternalToolTask *task = new LaunchExternalToolTask(execString);
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_onTaskFinishied()));
    return task;
}

void ExternalProcessWorker::sl_onTaskFinishied() {
    Task *t = static_cast<Task*>(sender());
    if(output && t->isFinished() && !t->hasError()) {
        QMap<QString, DataConfig>::iterator i;
        QVariantMap v;
        for(i = outputUrls.begin(); i != outputUrls.end(); i++) {
            DataConfig cfg = i.value();
            QString url = i.key();

            DocumentFormat *f = AppContext::getDocumentFormatRegistry()->getFormatById(cfg.format);
            IOAdapterFactory *io = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            TaskStateInfo ts;
            Document * d = f->loadDocument(io, url,ts, QVariantMap());

            if(d == NULL) {
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

            if(cfg.type == BaseTypes::DNA_SEQUENCE_TYPE()->getId()){
                DNASequenceObject *obj = static_cast<DNASequenceObject *>(d->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded).first());
                DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(cfg.type);
                v[WorkflowUtils::getSlotDescOfDatatype(dataType).getId()] = qVariantFromValue<DNASequence>(obj->getDNASequence());
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
                    DNASequenceObject *seqObj = static_cast<DNASequenceObject *>(d->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded).first());
                    DNASequence seq = seqObj->getSequence();
                    seq.alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::RAW());
                    v[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = qVariantFromValue<DNASequence>(seq);
                }
                if(!d->findGObjectByType(GObjectTypes::ANNOTATION_TABLE, UOF_LoadedAndUnloaded).isEmpty()) {
                    AnnotationTableObject *obj = static_cast<AnnotationTableObject *>(d->findGObjectByType(GObjectTypes::ANNOTATION_TABLE, UOF_LoadedAndUnloaded).first());
                    QList<SharedAnnotationData> list;
                    foreach(Annotation* a, obj->getAnnotations()) {
                        list << a->data();
                    }
                    v[BaseSlots::ANNOTATION_TABLE_SLOT().getId()] = qVariantFromValue(list);
                }
                
            }
            QFile::remove(url);
        }

        outputUrls.clear();
        DataTypePtr dataType = WorkflowEnv::getDataTypeRegistry()->getById(OUTPUT_PORT_TYPE + cfg->name);
        
        output->put(Message(dataType, v));
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
    foreach(const DataConfig& input, cfg->inputs) {
        inputs << ports.value(input.attrName);
    }

    output = ports.value(OUT_PORT_ID);
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
    externalProcess->start(execString);

    if(!externalProcess->waitForStarted(3000)) {
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
            foreach(Port *p, output->getLinks().keys()) {
                IntegralBusPort* ibp = qobject_cast<IntegralBusPort*>(p);
                Actor *dest = ibp->owner();
                destinations += tr("<u>%1</u>").arg(dest ? dest->getLabel() : unsetStr) + ",";
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
