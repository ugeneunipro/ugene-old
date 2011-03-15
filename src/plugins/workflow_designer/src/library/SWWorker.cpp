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

#include "SWWorker.h"
#include "FindWorker.h"

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/BaseAttributes.h>

#include <U2Core/DNASequence.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/MultiTask.h>
#include <U2Core/PluginModel.h>
#include <U2Core/FailTask.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/SequenceWalkerTask.h>
#include <U2Algorithm/SmithWatermanTaskFactoryRegistry.h>
#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2Algorithm/SWResultFilterRegistry.h>

#include "CoreLib.h"

/* TRANSLATOR U2::LocalWorkflow::SWWorker */
/* TRANSLATOR U2::LocalWorkflow::FindWorker */

namespace U2 {
namespace LocalWorkflow {

/**************************
 * SWWorkerFactory
 **************************/
static const QString NAME_ATTR("result-name");
static const QString PATTERN_ATTR("pattern");
static const QString SCORE_ATTR("min-score");
static const QString MATRIX_ATTR("matrix");
static const QString AMINO_ATTR("amino");
static const QString ALGO_ATTR("algorithm");
static const QString FILTER_ATTR("filter-strategy");
static const QString GAPOPEN_ATTR("gap-open-score");
static const QString GAPEXT_ATTR("gap-ext-score");

const QString SWWorkerFactory::ACTOR_ID("ssearch");

static const QString PATTERN_DELIMITER(";");

void SWAlgoEditor::populate() {
    QStringList algoLst = AppContext::getSmithWatermanTaskFactoryRegistry()->getListFactoryNames();
    if (algoLst.isEmpty()) {
        return;
    }
    foreach(const QString& n, algoLst) {
        items.insert(n,n);
    }
    QList<Attribute*> lst = proto->getAttributes();
    foreach(Attribute* a, lst) {
        if (a->getId() == ALGO_ATTR) {
            a->setAttributeValue(algoLst.first());
            break;
        }
    }
}

void SWWorkerFactory::init() {
    QList<PortDescriptor*> p;
    {
        Descriptor ind(BasePorts::IN_SEQ_PORT_ID(), SWWorker::tr("Input data"), 
            SWWorker::tr("An input sequence and set of regions to search in."));
        Descriptor oud(BasePorts::OUT_ANNOTATIONS_PORT_ID(), SWWorker::tr("Pattern annotations"), SWWorker::tr("Found regions"));
        
        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        p << new PortDescriptor(ind, DataTypePtr(new MapDataType("sw.sequence", inM)), true);
        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
        p << new PortDescriptor(oud, DataTypePtr(new MapDataType("sw.annotations", outM)), false, true);
    }
    
    QStringList filterLst = AppContext::getSWResultFilterRegistry()->getFiltersIds();
    
    QList<Attribute*> a;
    {
        Descriptor nd(NAME_ATTR, FindWorker::tr("Annotate as"), SWWorker::tr("Name of the result annotations marking found regions."));
        Descriptor pd(PATTERN_ATTR, FindWorker::tr("Pattern"), 
            SWWorker::tr("A subsequence pattern to look for. Several patterns can be delimited with semicolon"));
        Descriptor scd(SCORE_ATTR, SWWorker::tr("Min score"), SWWorker::tr("The search stringency."));
        Descriptor ald(ALGO_ATTR, SWWorker::tr("Algorithm"), SWWorker::tr("Algorithm version."));
        Descriptor amd(AMINO_ATTR, FindWorker::tr("Search in translation"), SWWorker::tr("Translate a supplied nucleotide sequence to protein then search in the translated sequence."));
        Descriptor mxd(MATRIX_ATTR, SWWorker::tr("Scoring matrix"), SWWorker::tr("The scoring matrix."));
        Descriptor frd(FILTER_ATTR, SWWorker::tr("Filter results"), SWWorker::tr("Result filtering strategy."));
        Descriptor god(GAPOPEN_ATTR, SWWorker::tr("Gap open score"), SWWorker::tr("Gap open score."));
        Descriptor ged(GAPEXT_ATTR, SWWorker::tr("Gap ext score"), SWWorker::tr("Gap extension score."));
        
        a << new Attribute(nd, BaseTypes::STRING_TYPE(), true, "misc_feature");
        a << new Attribute(pd, BaseTypes::STRING_TYPE(), true);
        a << new Attribute(mxd, BaseTypes::STRING_TYPE(), true, QString("Auto"));
        a << new Attribute(ald, BaseTypes::STRING_TYPE(), true);
        a << new Attribute(frd, BaseTypes::STRING_TYPE(), false, filterLst.isEmpty() ? QString() : filterLst.first());
        a << new Attribute(scd, BaseTypes::NUM_TYPE(), false, 90);
        a << new Attribute(BaseAttributes::STRAND_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false, BaseAttributes::STRAND_BOTH());
        a << new Attribute(amd, BaseTypes::BOOL_TYPE(), false, false);
        a << new Attribute(god, BaseTypes::NUM_TYPE(), false, -10.);
        a << new Attribute(ged, BaseTypes::NUM_TYPE(), false, -1.);
    }
    
    Descriptor desc(ACTOR_ID, SWWorker::tr("Smith-Waterman search"), SWWorker::tr("Finds regions of similarity to the specified pattern in each input sequence (nucleotide or protein one). "
        "<p>Under the hood is the well-known Smith-Waterman algorithm for performing local sequence alignment."));
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    
    // delegatess
    QMap<QString, PropertyDelegate*> delegates;    
    {
        QVariantMap m; m["minimum"] = 1; m["maximum"] = 100; m["suffix"] = "%";
        delegates[SCORE_ATTR] = new SpinBoxDelegate(m);
    }    
    {
        QVariantMap m; m["maximum"] = -0.; m["minimum"]=-10000000.;
        delegates[GAPOPEN_ATTR] = new DoubleSpinBoxDelegate(m);
        m["maximum"] = -1.;
        delegates[GAPEXT_ATTR] = new DoubleSpinBoxDelegate(m);
    }    
    delegates[BaseAttributes::STRAND_ATTRIBUTE().getId()] = new ComboBoxDelegate(BaseAttributes::STRAND_ATTRIBUTE_VALUES_MAP());
    {
        QVariantMap m;   
        foreach(const QString& n, filterLst) {
            m.insert(n,n);
        } 
        delegates[FILTER_ATTR] = new ComboBoxDelegate(m);
    }
    {
        QVariantMap m; m.insert(SWWorker::tr("Auto"), QString("Auto"));
        QStringList lst = AppContext::getSubstMatrixRegistry()->getMatrixNames();
        foreach(const QString& n, lst) {
            m.insert(n,n);
        } 
        delegates[MATRIX_ATTR] = new ComboBoxDelegate(m);
    }
    
    SWAlgoEditor* aled = new SWAlgoEditor(proto);
    aled->connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), SLOT(populate()));
    delegates[ALGO_ATTR] = aled;
    proto->setEditor(new DelegateEditor(delegates));
    proto->setIconPath(":core/images/sw.png");
    proto->setPrompter(new SWPrompter());
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_BASIC(), proto);
    
    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new SWWorkerFactory());
}

static StrandOption getStrand(const QString & s) {
    QString str = s.toLower();
    if(BaseAttributes::STRAND_BOTH().startsWith(str)) {
        return StrandOption_Both;
    } else if(BaseAttributes::STRAND_DIRECT().startsWith(str)) {
        return StrandOption_DirectOnly;
    } else if(BaseAttributes::STRAND_COMPLEMENTARY().startsWith(str)) {
        return StrandOption_ComplementOnly;
    } else {
        bool ok = false;
        int num = str.toInt(&ok);
        if(ok && num >= 0) {
            return StrandOption(num);
        } else {
            return StrandOption_Both;
        }
    }
}

/**************************
 * SWPrompter
 **************************/
QString SWPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    //Actor* annProducer = input->getProducer(BioActorLibrary::FEATURE_TABLE_SLOT_ID);
    Actor* seqProducer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString seqName = tr("In each sequence from <u>%1</u>,").arg(seqProducer ? seqProducer->getLabel() : unsetStr);
    //QString annName = annProducer ? tr(" within a set of regions from <u>%1</u>").arg(annProducer->getLabel()) : "";

    SmithWatermanSettings cfg;
    cfg.strand = getStrand(getParameter(BaseAttributes::STRAND_ATTRIBUTE().getId()).value<QString>());
    cfg.percentOfScore = getParameter(SCORE_ATTR).toInt();
    //cfg.insDelAlg = getParameter(ALGO_ATTR).toBool();
    QString pattern = getRequiredParam(PATTERN_ATTR);

    QString strandName;
    switch (cfg.strand) {
    case StrandOption_Both: strandName = FindWorker::tr("both strands"); break;
    case StrandOption_DirectOnly: strandName = FindWorker::tr("direct strand"); break;
    case StrandOption_ComplementOnly: strandName = FindWorker::tr("complement strand"); break;
    default: break;
    }
    if (getParameter(AMINO_ATTR).toBool()) {
        strandName += tr(" of translated sequence");
    }

    QString resultName = getRequiredParam(NAME_ATTR);
    
    QString match = cfg.percentOfScore < 100 ? 
        tr("matches with <u>at least %1% score</u>").arg(cfg.percentOfScore) : tr("exact matches");

    //FIXME mention search algorithm?
    QString doc = tr("%1 find pattern <u>%2</u>."
        "<br>Look for <u>%3</u> in <u>%4</u>."
        "<br>Output the list of found regions annotated as <u>%5</u>.")
        .arg(seqName)
        .arg(pattern)
        .arg(match)
        .arg(strandName)
        .arg(resultName);
    
    return doc;
}

/**************************
 * SWWorker
 **************************/
SWWorker::SWWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {
}

void SWWorker::init() {
    input = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
}

bool SWWorker::isReady() {
    return (input && input->hasMessage());
}

Task* SWWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(input);
    SmithWatermanSettings cfg;
    
    // sequence
    DNASequence seq = inputMessage.getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<DNASequence>();
    if(seq.isNull()) {
        return new FailTask(tr("Null sequence supplied to Smith-Waterman: %1").arg(seq.getName()));
    }
    QString ptrnStr = actor->getParameter(PATTERN_ATTR)->getAttributeValue<QString>().toUpper();
    QByteArray ptrnBytes = QString(ptrnStr).remove(PATTERN_DELIMITER).toAscii();
    if(!seq.alphabet->containsAll(ptrnBytes.constData(), ptrnBytes.length())) {
        algoLog.error(tr("Incorrect value: pattern alphabet doesn't match sequence alphabet "));
        return new FailTask(tr("Pattern symbols not matching to alphabet"));
    }
    cfg.sqnc = QByteArray(seq.constData(), seq.length());
    cfg.globalRegion.length = seq.length();

    // score and gap model
    cfg.percentOfScore = actor->getParameter(SCORE_ATTR)->getAttributeValue<int>();
    if(cfg.percentOfScore < 0 || cfg.percentOfScore > 100){
        algoLog.error(tr("Incorrect value: score value must lay between 0 and 100"));
        return new FailTask(tr("Incorrect value: score value must lay between 0 and 100"));
    }
    
    cfg.gapModel.scoreGapExtd = actor->getParameter(GAPEXT_ATTR)->getAttributeValue<double>();
    cfg.gapModel.scoreGapOpen = actor->getParameter(GAPOPEN_ATTR)->getAttributeValue<double>();
    
    // filter
    QString filter = actor->getParameter(FILTER_ATTR)->getAttributeValue<QString>();
    cfg.resultFilter = AppContext::getSWResultFilterRegistry()->getFilter(filter);
    if(cfg.resultFilter == NULL){
        algoLog.error(tr("Incorrect value:  filter name incorrect, default value used")); //details level won't work
        cfg.resultFilter = AppContext::getSWResultFilterRegistry()->getFilter("none");
    }
    
    // annotation name
    QString resultName = actor->getParameter(NAME_ATTR)->getAttributeValue<QString>();
    if(resultName.isEmpty()){
        algoLog.error(tr("Incorrect value: result name is empty, default value used")); //details level won't work
        resultName = "misc_feature";
    }
    
    // translations
    cfg.strand = getStrand(actor->getParameter(BaseAttributes::STRAND_ATTRIBUTE().getId())->getAttributeValue<QString>());
    if (cfg.strand != StrandOption_DirectOnly/* && seq.alphabet->getType() == DNAAlphabet_NUCL*/) {
        QList<DNATranslation*> compTTs = AppContext::getDNATranslationRegistry()->
                                            lookupTranslation(seq.alphabet, DNATranslationType_NUCL_2_COMPLNUCL);
        if (!compTTs.isEmpty()) {
            cfg.complTT = compTTs.first();
        } else {
            algoLog.error(tr("Could not find complement translation for %1, searching only direct strand").arg(seq.getName()));
            cfg.strand = StrandOption_DirectOnly;
        }
    }
    if (actor->getParameter(AMINO_ATTR)->getAttributeValue<bool>()) {
        DNATranslationType tt = seq.alphabet->getType() == DNAAlphabet_NUCL ? DNATranslationType_NUCL_2_AMINO : DNATranslationType_RAW_2_AMINO;
        QList<DNATranslation*> TTs = AppContext::getDNATranslationRegistry()->lookupTranslation(seq.alphabet, tt);
        if (!TTs.isEmpty()) {
            cfg.aminoTT = TTs.first(); //FIXME let user choose or use hints ?
        }
    }
    
    // scoring matrix
    QString mtrx = actor->getParameter(MATRIX_ATTR)->getAttributeValue<QString>();
    if(mtrx.isEmpty()){
        mtrx = "Auto";
    }
    cfg.pSm = AppContext::getSubstMatrixRegistry()->getMatrix(mtrx);
    if(cfg.pSm.getName().isEmpty() && mtrx.toLower() != "auto") {
        algoLog.details(tr("Invalid value: weight matrix with given name not exists"));
        return new FailTask(tr("Invalid value: weight matrix with given name not exists"));
    }
    if (cfg.pSm.isEmpty()) {
        QString matrixName;
        QStringList lst = AppContext::getSubstMatrixRegistry()->selectMatrixNamesByAlphabet(seq.alphabet);
        if (!lst.isEmpty()) {
            matrixName = lst.first();
            cfg.pSm = AppContext::getSubstMatrixRegistry()->getMatrix(matrixName);
        }
        if(cfg.pSm.isEmpty()) {
            return new FailTask(tr("Can't find weight matrix name: '%1'!").arg(matrixName.isEmpty() ? tr("<empty>") : matrixName));
        }
    }
    if(mtrx.toLower() != "auto") {
        if(seq.alphabet->getType() != cfg.pSm.getAlphabet()->getType()) {
            return new FailTask(tr("Wrong matrix selected. Alphabets do not match"));
        }
    }
    
    // algorithm
    QString algName = actor->getParameter(ALGO_ATTR)->getAttributeValue<QString>();
    SmithWatermanTaskFactory* algo = AppContext::getSmithWatermanTaskFactoryRegistry()->getFactory(algName);
    if (!algo) {
        return new FailTask(tr("SmithWaterman algorithm not found: %1").arg(algName));
    }
    
    // for each pattern run smith-waterman
    QStringList ptrnStrList = ptrnStr.split(PATTERN_DELIMITER, QString::SkipEmptyParts);
    if(ptrnStrList.isEmpty()) {
        algoLog.error(tr("Incorrect value: search pattern, pattern is empty"));
        return new FailTask(tr("Incorrect value: search pattern, pattern is empty"));
    }
    QList<Task*> subs;
    foreach(const QString & p, ptrnStrList) {
        assert(!p.isEmpty());
        SmithWatermanSettings config(cfg);
        config.ptrn = p.toAscii();
        
        SmithWatermanReportCallbackImpl* rcb = new SmithWatermanReportCallbackImpl( NULL, resultName, QString());
        config.resultCallback = rcb;
        config.resultListener = new SmithWatermanResultListener(); //FIXME: where to delete?
        
        Task * swTask = algo->getTaskInstance(config, tr("smith_waterman_task"));
        rcb->setParent(swTask); // swTask will delete rcb
        callbacks.insert(swTask, rcb);
        patterns.insert(swTask, config.ptrn);
        subs << swTask;
    }
    assert(!subs.isEmpty());
    
    MultiTask * multiSw = new MultiTask(tr("Smith waterman subtasks"), subs);
    connect(new TaskSignalMapper(multiSw), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
    return multiSw;
}

void SWWorker::sl_taskFinished(Task* t) {
    QList<SharedAnnotationData> annData;
    MultiTask * multiSw = qobject_cast<MultiTask*>(t);
    assert(multiSw != NULL);
    QList<Task*> subs = multiSw->getTasks();
    assert(!subs.isEmpty());
    QStringList ptrns;
    foreach(Task * sub, subs) {
        SmithWatermanReportCallbackImpl* rcb = callbacks.take(sub);
        assert(rcb != NULL);
        if(rcb) {
            annData << rcb->getAnotations();
        }
        ptrns << patterns.value(sub);
    }
    
    assert(output != NULL);
    if(output) {
        QVariant v = qVariantFromValue<QList<SharedAnnotationData> >(annData);
        output->put(Message(BaseTypes::ANNOTATION_TABLE_TYPE(), v));
        if (input->isEnded()) {
            output->setEnded();
        }
        algoLog.info(tr("Found %1 matches of pattern '%2'").arg(annData.size()).arg(ptrns.join(PATTERN_DELIMITER)));
    }
}

bool SWWorker::isDone() {
    return !input || input->isEnded();
}

void SWWorker::cleanup() {
}

} //namespace LocalWorkflow
} //namespace U2
