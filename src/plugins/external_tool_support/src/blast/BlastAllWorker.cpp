/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "BlastAllWorker.h"
#include "TaskLocalStorage.h"
#include "BlastAllSupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/Log.h>
#include <U2Core/FailTask.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2AlphabetUtils.h>

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/CoreLibConstants.h>

#include <U2Designer/DelegateEditors.h>

namespace U2 {
namespace LocalWorkflow {

/****************************
 * BlastAllWorkerFactory
 ****************************/
const QString BlastAllWorkerFactory::ACTOR_ID("blast");

#define BLASTALL_PROGRAM_NAME   QString("blast-type")
#define BLASTALL_DATABASE_PATH  QString("db-path")
#define BLASTALL_DATABASE_NAME  QString("db-name")
#define BLASTALL_EXPECT_VALUE   QString("e-val")
#define BLASTALL_GROUP_NAME     QString("result-name")
#define BLASTALL_EXT_TOOL_PATH  QString("tool-path")
#define BLASTALL_TMP_DIR_PATH   QString("temp-dir")
#define BLASTALL_GAP_COSTS_VALUE       QString("gap-costs")
#define BLASTALL_MATCH_SCORES_VALUE    QString("match-scores")

//Additional options
#define BLASTALL_ORIGINAL_OUT   QString("blast-output") //path for output file
#define BLASTALL_OUT_TYPE       QString("type-output")  //original option -m 0-11
#define BLASTALL_GAPPED_ALN     QString("gapped-aln") //Perform gapped alignment (not available with tblastx)

//const QString USE_MEGABLAST("mega-blast");//
//const QString WORD_SIZE("word-size");//
//const QString GAP_COSTS("gap-cost");//gap-open and gap-ext costs
//const QString MATCH_SCORES("match-scores");//mismatchPenalty and matchReward scores
//const QString MATRIX("matrix");//

void BlastAllWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    Descriptor ind(BasePorts::IN_SEQ_PORT_ID(), BlastAllWorker::tr("Input sequence"), 
        BlastAllWorker::tr("Sequence for which annotations is searched."));
    Descriptor oud(BasePorts::OUT_ANNOTATIONS_PORT_ID(), BlastAllWorker::tr("Annotations"), BlastAllWorker::tr("Found annotations."));
    
    QMap<Descriptor, DataTypePtr> inM;
    inM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
    p << new PortDescriptor(ind, DataTypePtr(new MapDataType("blast.seq", inM)), true /*input*/);
    QMap<Descriptor, DataTypePtr> outM;
    outM[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("blast.seq", outM)), false /*input*/, true /*multi*/);
    
    Descriptor pn(BLASTALL_PROGRAM_NAME, BlastAllWorker::tr("Search type"),
                   BlastAllWorker::tr("Select type of BLAST searches."));
    Descriptor dp(BLASTALL_DATABASE_PATH, BlastAllWorker::tr("Database Path"),
                   BlastAllWorker::tr("Path with database files."));
    Descriptor dn(BLASTALL_DATABASE_NAME, BlastAllWorker::tr("Database Name"),
                   BlastAllWorker::tr("Base name for BLAST DB files."));
    Descriptor ev(BLASTALL_EXPECT_VALUE, BlastAllWorker::tr("Expected value"),
                   BlastAllWorker::tr("This setting specifies the statistical significance threshold for reporting matches against database sequences."));
    Descriptor gn(BLASTALL_GROUP_NAME, BlastAllWorker::tr("Annotate as"),
                   BlastAllWorker::tr("Name for annotations."));
    Descriptor etp(BLASTALL_EXT_TOOL_PATH, BlastAllWorker::tr("Tool Path"),
                   BlastAllWorker::tr("External tool path."));
    Descriptor tdp(BLASTALL_TMP_DIR_PATH, BlastAllWorker::tr("Temporary directory"),
                   BlastAllWorker::tr("Directory for temporary files."));

    Descriptor output(BLASTALL_ORIGINAL_OUT, BlastAllWorker::tr("BLAST output"),
                   BlastAllWorker::tr("Location of BLAST output file."));
    Descriptor outtype(BLASTALL_OUT_TYPE, BlastAllWorker::tr("BLAST output type"),
                   BlastAllWorker::tr("Type of BLAST output file."));
    Descriptor ga(BLASTALL_GAPPED_ALN, BlastAllWorker::tr("Gapped alignment"),
                   BlastAllWorker::tr("Perform gapped alignment."));

    Descriptor gc(BLASTALL_GAP_COSTS_VALUE, BlastAllWorker::tr("Gap costs"),
                   BlastAllWorker::tr("Cost to create and extend a gap in an alignment."));
    Descriptor ms(BLASTALL_MATCH_SCORES_VALUE, BlastAllWorker::tr("Match scores"),
                   BlastAllWorker::tr("Reward and penalty for matching and mismatching bases."));

//    Descriptor umb(USE_MEGABLAST, BlastAllWorker::tr("Use MEGABLAST"),
//                   BlastAllWorker::tr("Activates MEGABLAST algorithm for blastn search"));
//    Descriptor ws(WORD_SIZE, BlastAllWorker::tr("Word size"),
//                   BlastAllWorker::tr("Specifies the word size"
//                                      "<br><b>Note:</b>"
//                                      "Zero invokes default, 11 for blastn, 28 for megablast, 3 for all others. Only 2 and 3 are available for protein alignment. Word size range for blastn is 7 and up, for megablast 12 and up. "));
//    Descriptor gc(GAP_COSTS, BlastAllWorker::tr("Gap costs"),
//                   BlastAllWorker::tr("Specifies the gap opening and extension cost"));
//    Descriptor ms(MATCH_SCORES, BlastAllWorker::tr("Match scores"),
//                   BlastAllWorker::tr("Specifies penalty for a nucleotide mismatch and reward for a nucleotide match"));
//    Descriptor matrix(MATRIX, BlastAllWorker::tr("Matrix"),
//                   BlastAllWorker::tr("Specifies which protein scoring matrix to use"));

    a << new Attribute(pn, BaseTypes::STRING_TYPE(), true, QVariant("blastn"));
    a << new Attribute(dp, BaseTypes::STRING_TYPE(), true, QVariant(""));
    a << new Attribute(dn, BaseTypes::STRING_TYPE(), true, QVariant(""));
    a << new Attribute(etp, BaseTypes::STRING_TYPE(), true, QVariant("default"));
    a << new Attribute(tdp, BaseTypes::STRING_TYPE(), true, QVariant("default"));
    a << new Attribute(ev, BaseTypes::NUM_TYPE(), false, QVariant(10.00));
    a << new Attribute(gn, BaseTypes::STRING_TYPE(), false, QVariant("blast_result"));

    Attribute* gaAttr= new Attribute(ga, BaseTypes::BOOL_TYPE(), false, QVariant(true));
    gaAttr->addRelation(new VisibilityRelation(BLASTALL_PROGRAM_NAME, "blastn"));
    gaAttr->addRelation(new VisibilityRelation(BLASTALL_PROGRAM_NAME, "blastp"));
    gaAttr->addRelation(new VisibilityRelation(BLASTALL_PROGRAM_NAME, "blastx"));
    gaAttr->addRelation(new VisibilityRelation(BLASTALL_PROGRAM_NAME, "tblastn"));
    a << gaAttr;
//    Attribute* umbAttr= new Attribute(umb, BaseTypes::BOOL_TYPE(), false, QVariant(false));
//    umbAttr->addRelation(PROGRAM_NAME,"blastn");
//    a << umbAttr;
//    a << new Attribute(ws, BaseTypes::NUM_TYPE(), false, QVariant(0));

//    Attribute* msAttr = new Attribute(ms, BaseTypes::STRING_TYPE(), false, QVariant("default"));
//    msAttr->addRelation(PROGRAM_NAME,"blastn");
//    a << msAttr;
//    Attribute* matrixAttr= new Attribute(matrix, BaseTypes::STRING_TYPE(), false, QVariant("BLOSUM62"));
//    matrixAttr->addRelation(PROGRAM_NAME,"blastp");
//    matrixAttr->addRelation(PROGRAM_NAME,"blastx");
//    matrixAttr->addRelation(PROGRAM_NAME,"tblastn");
//    matrixAttr->addRelation(PROGRAM_NAME,"tblastx");
//    a << matrixAttr;
//    a << new Attribute(gc, BaseTypes::STRING_TYPE(), false, QVariant("default"));

    a << new Attribute(gc, BaseTypes::STRING_TYPE(), false, "2 2");

    Attribute* msAttr = new Attribute(ms, BaseTypes::STRING_TYPE(), false, "1 -3");
    QVariantMap scoresGapDependency = ExternalToolSupportUtils::getScoresGapDependencyMap();
    msAttr->addRelation(new ValuesRelation(BLASTALL_GAP_COSTS_VALUE, scoresGapDependency));
    a << msAttr;

    a << new Attribute(output, BaseTypes::STRING_TYPE(), false, QVariant(""));
    a << new Attribute(outtype, BaseTypes::STRING_TYPE(), false, QVariant("7"));

    Descriptor desc(ACTOR_ID, BlastAllWorker::tr("Local BLAST Search"),
        BlastAllWorker::tr("Finds annotations for DNA sequence in local database."));
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    QMap<QString, PropertyDelegate*> delegates;

    {
        QVariantMap m;
        m["blastn"] = "blastn";
        m["blastp"] = "blastp";
        m["blastx"] = "blastx";
        m["tblastn"] = "tblastn";
        m["tblastx"] = "tblastx";
        delegates[BLASTALL_PROGRAM_NAME] = new ComboBoxDelegate(m);
    }
    {
        QVariantMap m;
        m["minimum"] = 0.000001;
        m["maximum"] = 100000;
        m["singleStep"] = 1.0;
        m["decimals"] = 6;
        delegates[BLASTALL_EXPECT_VALUE] = new DoubleSpinBoxDelegate(m);
    }
    {
        QVariantMap m;
        m["use"] = true;
        m["not use"] = false;
        delegates[BLASTALL_GAPPED_ALN] = new ComboBoxDelegate(m);
    }
    delegates[BLASTALL_DATABASE_PATH] = new URLDelegate("", "Database Directory", false, true, false);
    delegates[BLASTALL_ORIGINAL_OUT] = new URLDelegate("", "out file", false, false);
    delegates[BLASTALL_EXT_TOOL_PATH] = new URLDelegate("", "executable", false, false, false);
    delegates[BLASTALL_TMP_DIR_PATH] = new URLDelegate("", "TmpDir", false, true);

//    {
//        QVariantMap m;
//        m["default"] = "default";
//        m["1 -4"] = "1 -4";
//        m["1 -3"] = "1 -3";
//        m["1 -2"] = "1 -2";
//        m["1 -1"] = "1 -1";
//        m["2 -7"] = "2 -7";
//        m["2 -5"] = "2 -5";
//        m["2 -3"] = "2 -3";
//        m["4 -5"] = "4 -5";
//        m["5 -4"] = "5 -4";
//        delegates[MATCH_SCORES] = new ComboBoxDelegate(m);
//    }
//    {
//        QVariantMap m;
//        m["default"] = "default";
//        //if selected blastn
//        m["0 2 - with scores (1 -4), (1 -3), (1 -2), (1 -1)"] = "0 2";
//        m["0 4 - with scores (2 -7), (2 -5), (2 -3)"] = "0 4";
//        m["1 1 - with scores (1 -4), (1 -3), (1 -2)"] = "1 1";
//        m["1 2 - with scores (1 -4), (1 -3), (1 -2), (1 -1)"] = "1 2";
//        m["2 1 - with scores (1 -4), (1 -3), (1 -2), (1 -1)"] = "2 1";
//        m["2 2 - with scores (1 -4), (1 -3), (1 -2), (1 -1), (2 -7), (2 -5), (2 -3)"] = "2 2";
//        m["2 4 - with scores (2 -7), (2 -5), (2 -3)"] = "2 4";
//        m["3 1 - with scores (1 -2), (1 -1)"] = "3 1";
//        m["3 2 - with scores (1 -1)"] = "3 2";
//        m["3 3 - with scores (2 -3)"] = "3 3";
//        m["3 5 - with scores (4 -5), (5 -4)"] = "3 5";
//        m["4 1 - with scores (1 -1)"] = "4 1";
//        m["4 2 - with scores (1 -1), (2 -7), (2 -5), (2 -3)"] = "4 2";
//        m["4 4 - with scores (2 -7), (2 -5), (2 -3)"] = "4 4";
//        m["4 5 - with scores (4 -5), (5 -4)"] = "4 5";
//        m["5 2 - with scores (2 -3)"] = "5 2";
//        m["5 5 - with scores (4 -5), (5 -4)"] = "5 5";
//        m["6 2 - with scores (2 -3)"] = "6 2";
//        m["6 4 - with scores (2 -3)"] = "6 4";
//        m["6 5 - with scores (4 -5), (5 -4)"] = "6 5";
//        m["12 8 - with scores (4 -5), (5 -4)"] = "12 8";
//        //else
//        m["9 1 (PAM30, BLOSUM80)"] = "9 1";
//        m["5 2 (PAM30)"] = "5 2";
//        m["6 2 (PAM30, PAM70, BLOSUM80)"] = "6 2";
//        m["7 2 (PAM30, PAM70, BLOSUM62, BLOSUM80)"] = "7 2";
//        m["8 1 (PAM30)"] = "8 1";
//        m["10 1 (PAM30, PAM70, BLOSUM62, BLOSUM80)"] = "10 1";
//        m["8 2 (PAM70, BLOSUM62, BLOSUM80)"] = "8 2";
//        m["9 2 (PAM70, BLOSUM62)"] = "9 2";
//        m["11 1 (PAM70, BLOSUM62, BLOSUM80)"] = "11 1";
//        m["15 2 (BLOSUM45)"] = "15 2";
//        m["10 3 (BLOSUM45)"] = "10 3";
//        m["11 3 (BLOSUM45)"] = "11 3";
//        m["12 3 (BLOSUM45)"] = "12 3";
//        m["12 2 (BLOSUM45)"] = "12 2";
//        m["13 2 (BLOSUM45)"] = "13 2";
//        m["14 2 (BLOSUM45)"] = "14 2";
//        m["16 2 (BLOSUM45)"] = "16 2";
//        m["15 1 (BLOSUM45)"] = "15 1";
//        m["16 1 (BLOSUM45)"] = "16 1";
//        m["17 1 (BLOSUM45)"] = "17 1";
//        m["18 1 (BLOSUM45)"] = "18 1";
//        m["19 1 (BLOSUM45)"] = "19 1";
//        m["12 1 (BLOSUM62)"] = "12 1";
//        delegates[GAP_COSTS] = new ComboBoxDelegate(m);
//    }
//    {
//        QVariantMap m;
//        m["BLOSUM62"] = "BLOSUM62";
//        m["BLOSUM45"] = "BLOSUM45";
//        m["BLOSUM80"] = "BLOSUM80";
//        m["PAM30"] = "PAM30";
//        m["PAM70"] = "PAM70";
//        delegates[MATRIX] = new ComboBoxDelegate(m);
//    }
    {
        QVariantMap m;
        m["traditional pairwise (-m 0)"] = 0;
//        m["query-anchored showing identities"] = 1;
//        m["query-anchored no identities"] = 2;
//        m["flat query-anchored, show identities"] = 3;
//        m["flat query-anchored, no identities"] = 4;
//        m["query-anchored no identities and blunt ends"] = 5;
//        m["flat query-anchored, no identities and blunt ends"] = 6;
        m["XML (-m 7)"] = 7;
        m["tabular without post search sorting (-m 8)"] = 8;
//        m["tabular with comment lines and post search sorting"] = 9;
//        m["ascii ASN"] = 10;
//        m["binary ASN"] = 11;
        delegates[BLASTALL_OUT_TYPE] = new ComboBoxDelegate(m);
    }

    {
        QVariantMap m;
        const QList <QString> matchValues = scoresGapDependency.keys();
        for (int i = 0; i < matchValues.size(); i++) {
            m[matchValues.at(i)] = matchValues.at(i);
        }
        delegates[BLASTALL_MATCH_SCORES_VALUE] = new ComboBoxDelegate(m);;
    }

    {
        const QVariantMap m = scoresGapDependency.value("1 -3").toMap();
        delegates[BLASTALL_GAP_COSTS_VALUE] = new ComboBoxDelegate(m);
    }

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new BlastAllPrompter());
    proto->setIconPath(":external_tool_support/images/ncbi.png");
    proto->addExternalTool(ET_BLASTALL, BLASTALL_EXT_TOOL_PATH);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_BASIC(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new BlastAllWorkerFactory());
}
/****************************
* BlastAllPrompter
****************************/
BlastAllPrompter::BlastAllPrompter(Actor* p) : PrompterBase<BlastAllPrompter>(p) {
}
QString BlastAllPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    Actor* producer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);
    QString doc = tr("For sequence %1 find annotations in database <u>%2</u>.")
        .arg(producerName).arg(getHyperlink(BLASTALL_DATABASE_NAME, getRequiredParam(BLASTALL_DATABASE_NAME)));

    return doc;
}
/****************************
* BlastAllWorker
****************************/
BlastAllWorker::BlastAllWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {
}

void BlastAllWorker::init() {
    input = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
}

Task* BlastAllWorker::tick() {
    if (input->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(input);
        if (inputMessage.isEmpty()) {
            output->transit();
            return NULL;
        }
        cfg.programName=actor->getParameter(BLASTALL_PROGRAM_NAME)->getAttributeValue<QString>(context);
        cfg.databaseNameAndPath=actor->getParameter(BLASTALL_DATABASE_PATH)->getAttributeValue<QString>(context) +"/"+
                                actor->getParameter(BLASTALL_DATABASE_NAME)->getAttributeValue<QString>(context);
        cfg.isDefaultCosts=true;
        cfg.isDefaultMatrix=true;
        cfg.isDefautScores=true;
        cfg.wordSize=0;
        cfg.isGappedAlignment=actor->getParameter(BLASTALL_GAPPED_ALN)->getAttributeValue<bool>(context);
        cfg.expectValue=actor->getParameter(BLASTALL_EXPECT_VALUE)->getAttributeValue<double>(context);
        cfg.groupName=actor->getParameter(BLASTALL_GROUP_NAME)->getAttributeValue<QString>(context);
        if(cfg.groupName.isEmpty()){
            cfg.groupName="blast_result";
        }


        QString path=actor->getParameter(BLASTALL_EXT_TOOL_PATH)->getAttributeValue<QString>(context);
        if(QString::compare(path, "default", Qt::CaseInsensitive) != 0){
            AppContext::getExternalToolRegistry()->getByName(ET_BLASTALL)->setPath(path);
        }
        path=actor->getParameter(BLASTALL_TMP_DIR_PATH)->getAttributeValue<QString>(context);
        if(QString::compare(path, "default", Qt::CaseInsensitive) != 0){
            AppContext::getAppSettings()->getUserAppsSettings()->setUserTemporaryDirPath(path);
        }

        SharedDbiDataHandler seqId = inputMessage.getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<SharedDbiDataHandler>();
        std::auto_ptr<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
        if (NULL == seqObj.get()) {
            return NULL;
        }
        DNASequence seq = seqObj->getWholeSequence();

        if( seq.length() < 1) {
            return new FailTask(tr("Empty sequence supplied to BLAST"));
        }
        cfg.querySequence=seq.seq;

        const DNAAlphabet *alp = U2AlphabetUtils::findBestAlphabet(seq.seq);
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
        cfg.outputType=actor->getParameter(BLASTALL_OUT_TYPE)->getAttributeValue<int>(context);
        cfg.outputOriginalFile=getValue<QString>(BLASTALL_ORIGINAL_OUT);
        if(cfg.outputType != 7 && cfg.outputOriginalFile.isEmpty()){
            return new FailTask(tr("Not selected BLAST output file"));
        }

        if(cfg.programName == "blastn"){
            cfg.megablast = true;
            cfg.wordSize = 28;
            cfg.windowSize = 0;
        }else{
            cfg.megablast = false;
            cfg.wordSize = 3;
            cfg.windowSize  = 40;
        }
        //set X dropoff values
        if(cfg.programName == "blastn"){
            cfg.xDropoffFGA = 100;
            cfg.xDropoffGA = 20;
            cfg.xDropoffUnGA = 10;
        }else if (cfg.programName == "tblastx"){
            cfg.xDropoffFGA = 0;
            cfg.xDropoffGA = 0;
            cfg.xDropoffUnGA = 7;
        }else{
            cfg.xDropoffFGA = 25;
            cfg.xDropoffGA = 15;
            cfg.xDropoffUnGA = 7;
        }

        QString gapCosts = getValue<QString>(BLASTALL_GAP_COSTS_VALUE);
        cfg.gapOpenCost = gapCosts.split(" ").at(0).toInt();
        cfg.gapExtendCost = gapCosts.split(" ").at(1).toInt();
        QString matchScores = getValue<QString>(BLASTALL_MATCH_SCORES_VALUE);
        cfg.matchReward = matchScores.split(" ").at(0).toInt();
        cfg.mismatchPenalty = matchScores.split(" ").at(1).toInt();

        BlastAllSupportTask* t = new BlastAllSupportTask(cfg);
        t->addListeners(createLogListeners());
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    } else if (input->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void BlastAllWorker::sl_taskFinished() {
    BlastAllSupportTask* t = qobject_cast<BlastAllSupportTask*>(sender());
    if (t->getState() != Task::State_Finished || t->isCanceled() || t->hasError()){
        return;
    }

    if(output) {
        QList<SharedAnnotationData> res = t->getResultedAnnotations();
        QString annName = actor->getParameter(BLASTALL_GROUP_NAME)->getAttributeValue<QString>(context);
        if(!annName.isEmpty()) {
            for(int i = 0; i<res.count();i++) {
                res[i]->name = annName;
            }
        }
        QVariant v = qVariantFromValue<QList<SharedAnnotationData> >(res);
        output->put(Message(BaseTypes::ANNOTATION_TABLE_TYPE(), v));
    }
}

void BlastAllWorker::cleanup() {
}

} //namespace LocalWorkflow
} //namespace U2
