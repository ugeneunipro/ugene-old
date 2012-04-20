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

#include "RemoteBLASTWorker.h"

#include <U2Core/Log.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/FailTask.h>
#include <U2Core/U2AlphabetUtils.h>

#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>

#include <U2Designer/DelegateEditors.h>

namespace U2 {
namespace LocalWorkflow {


const QString RemoteBLASTWorkerFactory::ACTOR_ID("blast-ncbi");

const QString DATABASE("db");
const QString EXPECT("e-val");
const QString MAX_HITS("max-hits");
const QString SHORT_SEQ("short-sequence");
const QString ANNOTATION_NAME("result-name");
const QString ORIGINAL_OUT("blast-output");

void RemoteBLASTWorkerFactory::init() {
    QList<PortDescriptor*> p; 
    QList<Attribute*> a;
    Descriptor ind(BasePorts::IN_SEQ_PORT_ID(), RemoteBLASTWorker::tr("Input sequence"), 
        RemoteBLASTWorker::tr("The sequence to search the annotations for"));
    Descriptor outd(BasePorts::OUT_ANNOTATIONS_PORT_ID(), RemoteBLASTWorker::tr("Annotations"), 
        RemoteBLASTWorker::tr("Found annotations"));

    QMap<Descriptor, DataTypePtr> inM;
    inM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
    p << new PortDescriptor(ind, DataTypePtr(new MapDataType("blast.ncbi.sequence", inM)), true);
    QMap<Descriptor, DataTypePtr> outM;
    outM[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
    p << new PortDescriptor(outd, DataTypePtr(new MapDataType("blast.ncbi.annotations", outM)), false, true);
    
    Descriptor db(DATABASE,RemoteBLASTWorker::tr("Database"),
        RemoteBLASTWorker::tr("Select the database to search through. Available databases are blastn, blastp and cdd"));
    Descriptor evalue(EXPECT,RemoteBLASTWorker::tr("Expected value"),
        RemoteBLASTWorker::tr("This parameter specifies the statistical significance threshold of reporting matches against the database sequences."));
    Descriptor hits(MAX_HITS,RemoteBLASTWorker::tr("Max hits"),
        RemoteBLASTWorker::tr("Maximum number of hits."));
    Descriptor short_seq(SHORT_SEQ,RemoteBLASTWorker::tr("Short sequence"),
        RemoteBLASTWorker::tr("Optimize search for short sequences."));
    Descriptor annotateAs(ANNOTATION_NAME,RemoteBLASTWorker::tr("Annotate as"),
        RemoteBLASTWorker::tr("Name for annotations"));
    Descriptor output(ORIGINAL_OUT, RemoteBLASTWorker::tr("BLAST output"),
        RemoteBLASTWorker::tr("Location of BLAST output file. This parameter insignificant for cdd search."));

    a << new Attribute(db,BaseTypes::STRING_TYPE(),true,"ncbi-blastn");
    a << new Attribute(evalue,BaseTypes::STRING_TYPE(),false,10);
    a << new Attribute(hits,BaseTypes::NUM_TYPE(),false,10);
    a << new Attribute(short_seq,BaseTypes::BOOL_TYPE(),false,false);
    a << new Attribute(annotateAs,BaseTypes::STRING_TYPE(),false);
    a << new Attribute(output, BaseTypes::STRING_TYPE(),false);

    Descriptor desc(ACTOR_ID, RemoteBLASTWorker::tr("Remote BLAST"), 
        RemoteBLASTWorker::tr("Finds annotations for DNA sequence in remote database")
        );
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->addSlotRelation(BasePorts::IN_SEQ_PORT_ID(), BaseSlots::DNA_SEQUENCE_SLOT().getId(),
        BasePorts::OUT_ANNOTATIONS_PORT_ID(), BaseSlots::ANNOTATION_TABLE_SLOT().getId());
    QMap<QString, PropertyDelegate*> delegates; 

    {
        QVariantMap m;
        m["minimum"] = 1;
        m["maximum"] = 500;
        delegates[MAX_HITS] = new SpinBoxDelegate(m);
    }

    {
        QVariantMap m;
        m["ncbi-blastn"] = "ncbi-blastn";
        m["ncbi-blastp"] = "ncbi-blastp";
        m["ncbi-cdd"] = "ncbi-cdd";
        delegates[DATABASE] = new ComboBoxDelegate(m);
    }

    {
        QVariantMap m;
        m["minimum"] = 0.000001;
        m["maximum"] = 100000;
        m["singleStep"] = 1.0;
        m["decimals"] = 6;
        delegates[EXPECT] = new DoubleSpinBoxDelegate(m);
    }

    delegates[ORIGINAL_OUT] = new URLDelegate("(*.xml)","xml file");

    proto->setPrompter(new RemoteBLASTPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    proto->setIconPath(":remote_blast/images/remote_db_request.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_BASIC(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new RemoteBLASTWorkerFactory());
}

QString RemoteBLASTPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    Actor* producer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);

    QString doc = tr("For sequence %1 find annotations in database <u>%2</u>")
        .arg(producerName).arg(getHyperlink(DATABASE, getRequiredParam(DATABASE)));
    return doc;
}

void RemoteBLASTWorker::init() {
    input = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
}

Task* RemoteBLASTWorker::tick() {
    if((actor->getParameter(ANNOTATION_NAME)->getAttributeValue<QString>(context)).isEmpty()){
        algoLog.details(tr("Annotations name is empty, default name used"));
    }
    
    if (input->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(input);
        if (inputMessage.isEmpty()) {
            output->transit();
            return NULL;
        }
        //cfg.minrl = 0;
        //cfg.maxrl = 3000;
        cfg.dbChoosen = actor->getParameter(DATABASE)->getAttributeValue<QString>(context).split("-").last();
        cfg.aminoT = NULL;

        int evalue = actor->getParameter(EXPECT)->getAttributeValue<int>(context);
        int maxHits = actor->getParameter(MAX_HITS)->getAttributeValue<int>(context);
        bool shortSeq = actor->getParameter(SHORT_SEQ)->getAttributeValue<bool>(context);

        if(evalue <= 0 ){
            algoLog.error(tr("Incorrect value for 'e-value' parameter, default value passed to schema"));
            evalue = 10;
        }

        if(cfg.dbChoosen == "cdd") {
            cfg.params = "db=cdd";
            addParametr(cfg.params,ReqParams::cdd_hits,maxHits);
            addParametr(cfg.params,ReqParams::cdd_eValue,evalue);
        }
        else {
            cfg.params = "CMD=Put";
            addParametr(cfg.params,ReqParams::database,"nr");
            addParametr(cfg.params, ReqParams::program, cfg.dbChoosen);
            QString filter;
            QString wordSize;
            if(shortSeq) {
                evalue = 1000;
                filter = "";
                if(cfg.dbChoosen == "blastn") {
                    addParametr(cfg.params, ReqParams::wordSize, 7);
                }
            }
            else {
                addParametr(cfg.params, ReqParams::filter, "L");
            }
            addParametr(cfg.params, ReqParams::expect, evalue);
            
            addParametr(cfg.params, ReqParams::hits, maxHits);
        }
        SharedDbiDataHandler seqId = inputMessage.getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<SharedDbiDataHandler>();
        std::auto_ptr<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
        if (NULL == seqObj.get()) {
            return NULL;
        }
        DNASequence seq = seqObj->getWholeSequence();
        
        seq.info.clear();
        DNAAlphabet *alp = U2AlphabetUtils::findBestAlphabet(seq.seq);
        /*if(seq.length()>MAX_BLAST_SEQ_LEN) {
            log.error(tr("The sequence is too long"));
            return NULL;
        }*/
        if(alp == AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::AMINO_DEFAULT())) {
            if(cfg.dbChoosen == "blastn") {
                algoLog.details(tr("Selected nucleotide database"));
                return NULL;
            }
        }
        else {
            if(cfg.dbChoosen != "blastn") {
                algoLog.details(tr("Selected amino acid database"));
                return NULL;
            }
        }
        cfg.query = seq.seq;
        cfg.retries = 60;
        cfg.filterResult = 0;
        Task* t = new RemoteBLASTTask(cfg);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    } else if (input->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void RemoteBLASTWorker::sl_taskFinished() {
    RemoteBLASTTask * t = qobject_cast<RemoteBLASTTask*>(sender());
    if (t->getState() != Task::State_Finished || t->hasError()) {
        return;
    }

    if(output) {
        if(actor->getParameter(DATABASE)->getAttributeValue<QString>(context) != "ncbi-cdd") {
            QString url = actor->getParameter(ORIGINAL_OUT)->getAttributeValue<QString>(context);
            if(!url.isEmpty()) {
                IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( BaseIOAdapters::LOCAL_FILE );
                IOAdapter * io = iof->createIOAdapter();
                if(io->open( url, IOAdapterMode_Write )) {
                    QByteArray output = t->getOutputFile();
                    io->writeBlock(output);
                    io->close();
                }
            }
        }

        QList<SharedAnnotationData> res = t->getResultedAnnotations();
        QString annName = actor->getParameter(ANNOTATION_NAME)->getAttributeValue<QString>(context);
        if(!annName.isEmpty()) {
            for(int i = 0; i<res.count();i++) {
                res[i]->name = annName;
            }
        }
        QVariant v = qVariantFromValue<QList<SharedAnnotationData> >(res);
        output->put(Message(BaseTypes::ANNOTATION_TABLE_TYPE(), v));
    }
}

}
}
