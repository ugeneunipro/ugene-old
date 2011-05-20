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

#include "BlastPlusWorker.h"
#include "TaskLocalStorage.h"
#include "BlastPlusSupport.h"
#include "BlastNPlusSupportTask.h"
#include "BlastPPlusSupportTask.h"
#include "BlastXPlusSupportTask.h"
#include "TBlastNPlusSupportTask.h"
#include "TBlastXPlusSupportTask.h"

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/Log.h>
#include <U2Core/FailTask.h>

namespace U2 {
namespace LocalWorkflow {

/****************************
 * BlastAllWorkerFactory
 ****************************/
const QString BlastPlusWorkerFactory::ACTOR_ID("blast-plus");
//const QString PROGRAM_NAME("programName");
//const QString DATABASE_PATH("databasePath");
//const QString DATABASE_NAME("databaseName");
//const QString EXPECT_VALUE("expectValue");
//const QString GROUP_NAME("groupName");
//const QString ORIGINAL_OUT("Blast_output");
//const QString EXT_TOOL_PATH("extToolPath");
//const QString TMP_DIR_PATH("tmpDirPath");

const QString PROGRAM_NAME("blast-type");
const QString DATABASE_PATH("db-path");
const QString DATABASE_NAME("db-name");
const QString EXPECT_VALUE("e-val");
const QString GROUP_NAME("result-name");
//const QString ORIGINAL_OUT("Blast_output");
const QString EXT_TOOL_PATH("tool-path");
const QString TMP_DIR_PATH("temp-dir");


void BlastPlusWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    Descriptor ind(BasePorts::IN_SEQ_PORT_ID(), BlastPlusWorker::tr("Input sequence"), 
        BlastPlusWorker::tr("Sequence for which annotations is searched."));
    Descriptor oud(BasePorts::OUT_ANNOTATIONS_PORT_ID(), BlastPlusWorker::tr("Annotations"), BlastPlusWorker::tr("Found annotations."));
    
    QMap<Descriptor, DataTypePtr> inM;
    inM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
    p << new PortDescriptor(ind, DataTypePtr(new MapDataType("blast.plus.seq", inM)), true /*input*/);
    QMap<Descriptor, DataTypePtr> outM;
    outM[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("blast.plus.annotations", outM)), false /*input*/, true /*multi*/);
    
    Descriptor pn(PROGRAM_NAME, BlastPlusWorker::tr("Search type"),
                   BlastPlusWorker::tr("Select type of BLAST+ searches"));
    Descriptor dp(DATABASE_PATH, BlastPlusWorker::tr("Database Path"),
                   BlastPlusWorker::tr("Path with database files"));
    Descriptor dn(DATABASE_NAME, BlastPlusWorker::tr("Database Name"),
                   BlastPlusWorker::tr("Base name for BLAST+ DB files"));
    Descriptor ev(EXPECT_VALUE, BlastPlusWorker::tr("Expected value"),
                   BlastPlusWorker::tr("This setting specifies the statistical significance threshold for reporting matches against database sequences."));
    Descriptor gn(GROUP_NAME, BlastPlusWorker::tr("Annotate as"),
                   BlastPlusWorker::tr("Name for annotations"));
//    Descriptor output(ORIGINAL_OUT, BlastAllWorker::tr("BLAST output"),
//                   BlastAllWorker::tr("Location of BLAST output file."));
    Descriptor etp(EXT_TOOL_PATH, BlastPlusWorker::tr("Tool Path"),
                   BlastPlusWorker::tr("External tool path"));
    Descriptor tdp(TMP_DIR_PATH, BlastPlusWorker::tr("Temporary directory"),
                   BlastPlusWorker::tr("Directory for temporary files"));

    a << new Attribute(pn, BaseTypes::STRING_TYPE(), true, QVariant("blastn"));
    a << new Attribute(dp, BaseTypes::STRING_TYPE(), true, QVariant(""));
    a << new Attribute(dn, BaseTypes::STRING_TYPE(), true, QVariant(""));
    a << new Attribute(ev, BaseTypes::NUM_TYPE(), false, QVariant(10.00));
    a << new Attribute(gn, BaseTypes::STRING_TYPE(), false, QVariant(""));
//    a << new Attribute(output, BaseTypes::STRING_TYPE(), false, QVariant(""));
    a << new Attribute(etp, BaseTypes::STRING_TYPE(), true, QVariant("default"));
    a << new Attribute(tdp, BaseTypes::STRING_TYPE(), true, QVariant("default"));

    Descriptor desc(ACTOR_ID, BlastPlusWorker::tr("Local BLAST+ search"),
        BlastPlusWorker::tr("Finds annotations for DNA sequence in local database"));
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

    QMap<QString, PropertyDelegate*> delegates;

    {
        QVariantMap m;
        m["blastn"] = "blastn";
        m["blastp"] = "blastp";
        m["blastx"] = "blastx";
        m["tblastn"] = "tblastn";
        m["tblastx"] = "tblastx";
        delegates[PROGRAM_NAME] = new ComboBoxDelegate(m);
    }
    {
        QVariantMap m;
        m["1e-100"] = 1e-100;
        m["1e-10"] = 1e-10;
        m["1"] = 1;
        m["10"] = 10;
        m["100"] = 100;
        m["1000"] = 1000;
        delegates[EXPECT_VALUE] = new ComboBoxDelegate(m);
    }
    delegates[DATABASE_PATH] = new URLDelegate("", "Database Directory", false, true);
//    delegates[ORIGINAL_OUT] = new URLDelegate("(*.xml)", "xml file", false);
    delegates[EXT_TOOL_PATH] = new URLDelegate("", "executable", false);
    delegates[TMP_DIR_PATH] = new URLDelegate("", "TmpDir", false, true);

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new BlastPlusPrompter());
    proto->setIconPath(":external_tool_support/images/ncbi.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_BASIC(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new BlastPlusWorkerFactory());
}

/****************************
* BlastPlusPrompter
****************************/
BlastPlusPrompter::BlastPlusPrompter(Actor* p) : PrompterBase<BlastPlusPrompter>(p) {
}
QString BlastPlusPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    Actor* producer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);
    QString doc = tr("For sequence <u>%1</u> find annotations in database <u>%2</u>")
        .arg(producerName).arg(getParameter(DATABASE_NAME).toString());

    return doc;
}
/****************************
* BlastPlusWorker
****************************/
BlastPlusWorker::BlastPlusWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {
}

void BlastPlusWorker::init() {
    input = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
}

bool BlastPlusWorker::isReady() {
    return (input && input->hasMessage());
}

Task* BlastPlusWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(input);
    cfg.programName=actor->getParameter(PROGRAM_NAME)->getAttributeValue<QString>();
    cfg.databaseNameAndPath=actor->getParameter(DATABASE_PATH)->getAttributeValue<QString>() +"/"+
                            actor->getParameter(DATABASE_NAME)->getAttributeValue<QString>();
    cfg.isDefaultCosts=true;
    cfg.isDefaultMatrix=true;
    cfg.isDefautScores=true;
    cfg.expectValue=actor->getParameter(EXPECT_VALUE)->getAttributeValue<double>();
    cfg.groupName=actor->getParameter(GROUP_NAME)->getAttributeValue<QString>();
    if(cfg.groupName.isEmpty()){
        cfg.groupName="blast result";
    }
    cfg.wordSize=0;

    QString path=actor->getParameter(EXT_TOOL_PATH)->getAttributeValue<QString>();
    if(QString::compare(path, "default", Qt::CaseInsensitive) != 0){
        if(cfg.programName == "blastn"){
            AppContext::getExternalToolRegistry()->getByName(BLASTN_TOOL_NAME)->setPath(path);
        }else if(cfg.programName == "blastp"){
            AppContext::getExternalToolRegistry()->getByName(BLASTP_TOOL_NAME)->setPath(path);
        }else if(cfg.programName == "blastx"){
            AppContext::getExternalToolRegistry()->getByName(BLASTX_TOOL_NAME)->setPath(path);
        }else if(cfg.programName == "tblastn"){
            AppContext::getExternalToolRegistry()->getByName(TBLASTN_TOOL_NAME)->setPath(path);
        }else if(cfg.programName == "tblastx"){
            AppContext::getExternalToolRegistry()->getByName(TBLASTX_TOOL_NAME)->setPath(path);
        }

    }
    path=actor->getParameter(TMP_DIR_PATH)->getAttributeValue<QString>();
    if(QString::compare(path, "default", Qt::CaseInsensitive) != 0){
        AppContext::getAppSettings()->getUserAppsSettings()->setTemporaryDirPath(path);
    }
    DNASequence seq = inputMessage.getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<DNASequence>();
    
    if( seq.length() < 1) {
        return new FailTask(tr("Empty sequence supplied to BLAST"));
    }
    cfg.querySequence=seq.seq;

    DNAAlphabet *alp = AppContext::getDNAAlphabetRegistry()->findAlphabet(seq.seq);
    cfg.alphabet=alp;
    //TO DO: Check alphabet
    if(seq.alphabet->isAmino()) {
        if(cfg.programName == "blastn" || cfg.programName == "blastx" || cfg.programName == "tblastx") {
            return new FailTask(tr("Selected BLAST search with nucleotide input sequence"));
        }
    }
    else {
        if(cfg.programName == "blastp" || cfg.programName == "tblastn") {
            return new FailTask(tr("Selected BLAST search with amino acid input sequence"));
        }
    }
    cfg.needCreateAnnotations=false;
    Task * t=NULL;
    if(cfg.programName == "blastn"){
        t = new BlastNPlusSupportTask(cfg);
    }else if(cfg.programName == "blastp"){
        t = new BlastPPlusSupportTask(cfg);
    }else if(cfg.programName == "blastx"){
        t = new BlastXPlusSupportTask(cfg);
    }else if(cfg.programName == "tblastn"){
        t = new TBlastNPlusSupportTask(cfg);
    }else if(cfg.programName == "tblastx"){
        t = new TBlastXPlusSupportTask(cfg);
    }
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    return t;
}

void BlastPlusWorker::sl_taskFinished() {
    BlastPlusSupportCommonTask* t = qobject_cast<BlastPlusSupportCommonTask*>(sender());
    if (t->getState() != Task::State_Finished) return;

    if(output) {
        QList<SharedAnnotationData> res = t->getResultedAnnotations();
        QString annName = actor->getParameter(GROUP_NAME)->getAttributeValue<QString>();
        if(!annName.isEmpty()) {
            for(int i = 0; i<res.count();i++) {
                res[i]->name = annName;
            }
        }
        QVariant v = qVariantFromValue<QList<SharedAnnotationData> >(res);
        output->put(Message(BaseTypes::ANNOTATION_TABLE_TYPE(), v));
        if (input->isEnded()) {
            output->setEnded();
        }
    }
}

bool BlastPlusWorker::isDone() {
    return !input || input->isEnded();
}

void BlastPlusWorker::cleanup() {
}

} //namespace LocalWorkflow
} //namespace U2
