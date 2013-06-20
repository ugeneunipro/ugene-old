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

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/MultiTask.h>
#include <U2Core/FailTask.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Algorithm/FindAlgorithmTask.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Formats/GenbankFeatures.h>
#include <U2Core/AnnotationTableObject.h>

#include "CoreLib.h"
#include "FindWorker.h"

/* TRANSLATOR U2::LocalWorkflow::FindWorker */

namespace U2 {
namespace LocalWorkflow {

/***************************
 * FindWorkerFactory
 ***************************/
static const QString NAME_ATTR("result-name");
static const QString PATTERN_ATTR("pattern");
static const QString PATTERN_FILE_ATTR("pattern_file");
static const QString USE_NAMES_ATTR("use-names");
static const QString ERR_ATTR("max-mismatches-num");
static const QString ALGO_ATTR("allow-ins-del");
static const QString AMINO_ATTR("amino");
static const QString AMBIGUOUS_ATTR("ambiguous");

const QString FindWorkerFactory::ACTOR_ID("search");

const QString PATTERN_DELIMITER(";");

void FindWorkerFactory::init() {

    QMap<Descriptor, DataTypePtr> m;
    m[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
    m[BaseSlots::TEXT_SLOT()] = BaseTypes::STRING_TYPE();
    DataTypePtr inSet(new MapDataType(Descriptor("regioned.sequence"), m));
    DataTypeRegistry* dr = WorkflowEnv::getDataTypeRegistry();
    assert(dr);
    dr->registerEntry(inSet);
    
    QList<PortDescriptor*> p;
    {
        Descriptor ind(BasePorts::IN_SEQ_PORT_ID(),
            FindWorker::tr("Input Data"),
            FindWorker::tr("An input sequence to search in."));

        Descriptor oud(BasePorts::OUT_ANNOTATIONS_PORT_ID(),
            FindWorker::tr("Pattern Annotations"),
            FindWorker::tr("The regions found."));

        p << new PortDescriptor(ind, inSet, true);
        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
        p << new PortDescriptor(oud, DataTypePtr(new MapDataType("find.annotations", outM)), false, true);
    }
    QList<Attribute*> a;
    {
        Descriptor nd(NAME_ATTR,
            FindWorker::tr("Annotate as"),
            FindWorker::tr("Name of the result annotations."));

        Descriptor pd(PATTERN_ATTR,
            FindWorker::tr("Pattern(s)"),
            FindWorker::tr("Semicolon-separated list of patterns to search for."));

        Descriptor pf(PATTERN_FILE_ATTR,
            FindWorker::tr("Pattern file"),
            FindWorker::tr("Load pattern from file in any sequence format or in newline-delimited format"));

        Descriptor un(USE_NAMES_ATTR,
            FindWorker::tr("Use pattern name"),
            FindWorker::tr("If patterns are loaded from a file, use names of pattern sequences as annotation names. The name from the parameters is used by default"));

        Descriptor ed(ERR_ATTR, FindWorker::tr("Max Mismatches"),
            FindWorker::tr("Maximum number of mismatches between a substring"
                " and a pattern."));

        Descriptor ald(ALGO_ATTR,
            FindWorker::tr("Allow Insertions/Deletions"),
            FindWorker::tr("Takes into account possibility of insertions/deletions"
                " when searching. By default substitutions are only considered."));
                
        Descriptor ambigd(AMBIGUOUS_ATTR,
            FindWorker::tr("Support ambiguous bases"),
            FindWorker::tr("Performs correct handling of ambiguous bases. When this option"
            " is activated insertions and deletions are not considered. "));


        Descriptor amd(AMINO_ATTR,
            FindWorker::tr("Search in Translation"),
            FindWorker::tr("Translates a supplied nucleotide sequence to protein"
            " and searches in the translated sequence."));
        
        a << new Attribute(nd, BaseTypes::STRING_TYPE(), true, "misc_feature");
        a << new Attribute(pd, BaseTypes::STRING_TYPE(), false);
        a << new Attribute(pf, BaseTypes::STRING_TYPE(), false);
        a << new Attribute(un, BaseTypes::BOOL_TYPE(), false, false);
        a << new Attribute(ed, BaseTypes::NUM_TYPE(), false, 0);
        a << new Attribute(BaseAttributes::STRAND_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false, BaseAttributes::STRAND_BOTH());
        a << new Attribute(ald, BaseTypes::BOOL_TYPE(), false, false);
        a << new Attribute(ambigd, BaseTypes::BOOL_TYPE(), false, false);
        a << new Attribute(amd, BaseTypes::BOOL_TYPE(), false, false);
        
    }
    
    Descriptor desc(ACTOR_ID,
        FindWorker::tr("Find Pattern"),
        FindWorker::tr("Searches regions in a sequence similar to a pattern"
            " sequence. Outputs a set of annotations."));

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    
    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap lenMap; 
        lenMap["minimum"] = QVariant(0); 
        lenMap["maximum"] = QVariant(INT_MAX);
        delegates[ERR_ATTR] = new SpinBoxDelegate(lenMap);
    }
    {
        delegates[PATTERN_FILE_ATTR] = new URLDelegate("", "File with patterns", false, false);
    }
    {
        delegates[BaseAttributes::STRAND_ATTRIBUTE().getId()] = new ComboBoxDelegate(BaseAttributes::STRAND_ATTRIBUTE_VALUES_MAP());
    }
    proto->setEditor(new DelegateEditor(delegates));
    
    proto->setIconPath( ":core/images/find_dialog.png" );
    proto->setPrompter(new FindPrompter());
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_BASIC(), proto);
    
    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new FindWorkerFactory());
}

static FindAlgorithmStrand getStrand(const QString & s) {
    QString str = s.toLower();
    if(BaseAttributes::STRAND_BOTH().startsWith(str)) {
        return FindAlgorithmStrand_Both;
    } else if(BaseAttributes::STRAND_DIRECT().startsWith(str)) {
        return FindAlgorithmStrand_Direct;
    } else if(BaseAttributes::STRAND_COMPLEMENTARY().startsWith(str)) {
        return FindAlgorithmStrand_Complement;
    } else {
        bool ok = false;
        int num = str.toInt(&ok);
        if(ok && num >= 0) {
            return FindAlgorithmStrand(num);
        } else {
            return FindAlgorithmStrand_Both;
        }
    }
}

/***************************
 * FindPrompter
 ***************************/
QString FindPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    Actor* seqProducer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    QString unsetStr = "<font color='red'>" + tr("unset") + "</font>";

    QString seqName;
    seqProducer ? (seqName = seqProducer->getLabel()) : (seqName = unsetStr);

    FindAlgorithmSettings cfg;
    cfg.strand = getStrand(getParameter(BaseAttributes::STRAND_ATTRIBUTE().getId()).value<QString>());
    cfg.maxErr = getParameter(ERR_ATTR).toInt();
    cfg.patternSettings = static_cast<FindAlgorithmPatternSettings>(getParameter(ALGO_ATTR).toInt());

    QString strandName;
    switch (cfg.strand) {
        case FindAlgorithmStrand_Both:
            strandName = FindWorker::tr("both strands");
            break;
        case FindAlgorithmStrand_Direct:
            strandName = FindWorker::tr("direct strand");
            break;
        case FindAlgorithmStrand_Complement:
            strandName = FindWorker::tr("complement strand");
            break;
        default:
            assert(false);
    }
    strandName = getHyperlink(BaseAttributes::STRAND_ATTRIBUTE().getId(), strandName);

    QString searchInTranslationSelected = "";
    if (getParameter(AMINO_ATTR).toBool()) {
        searchInTranslationSelected = "<u>" + tr("translated") + "</u>" + " ";
    }
    searchInTranslationSelected = getHyperlink(AMINO_ATTR, searchInTranslationSelected);

    QString resultName = getHyperlink(NAME_ATTR, getRequiredParam(NAME_ATTR));

    QString matches;
    if (0 == cfg.maxErr) {
        matches = tr("A substring must %1.")
            .arg(getHyperlink(ERR_ATTR, tr("match a pattern exactly")));
    } else {
        matches = tr("Maximum number of mismatches is %1.")
            .arg(getHyperlink(ERR_ATTR, cfg.maxErr));
    }

    QString patternStr;
    Actor* patternProd = input->getProducer(BaseSlots::TEXT_SLOT().getId());
    if (NULL == patternProd) {
        QString pattern = getHyperlink(PATTERN_ATTR, getRequiredParam(PATTERN_ATTR));
        patternStr = tr("<u>%1</u> pattern(s)").arg(pattern);
    } else {
        patternStr = tr("patterns from <u>%1</u>").arg(patternProd->getLabel());
    }

    QString patternFileStr;
    QString filePathParam = getParameter(PATTERN_FILE_ATTR).toString();
    if (!filePathParam.isEmpty()) {
        QString pattern = getHyperlink(PATTERN_FILE_ATTR, filePathParam);
        
        patternFileStr = tr(" and <u>%1</u>").arg(pattern);

        bool useNames = getParameter(USE_NAMES_ATTR).toBool();
        if (useNames){
            patternFileStr+=tr(" using pattern names");
        }
    }
    QString doc = tr("Searches regions in each sequence from <u>%1</u>"
        " similar to %2%3.<br/>%4<br/>Searches in"
        " <u>%5</u> of a %6sequence. Outputs the regions found"
        " annotated as <u>%7</u>.")
        .arg(seqName)
        .arg(patternStr)
        .arg(patternFileStr)
        .arg(matches)
        .arg(strandName)
        .arg(searchInTranslationSelected)
        .arg(resultName);

    return doc;
}

/***************************
 * FindWorker
 ***************************/
FindWorker::FindWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL)
,patternFileLoaded(false)
,useNames(false)
{

}

void FindWorker::init() {
    input = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
}

Task* FindWorker::tick() {
    if (!patternFileLoaded){
        QString patternFilePath = actor->getParameter(PATTERN_FILE_ATTR)->getAttributeValue<QString>(context);
        patternFileLoaded = true;
        if (!patternFilePath.isEmpty()){
            QList<Task*> subs;

            LoadPatternsFileTask * loadPatternTask = new LoadPatternsFileTask(patternFilePath);
            subs.append(loadPatternTask);

            MultiTask * multiFind = new MultiTask(tr("Load file with patterns"), subs);
            connect(new TaskSignalMapper(multiFind), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
            return multiFind;
        }
    }
    if (input->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(input);
        if (inputMessage.isEmpty()) {
            output->transit();
            return NULL;
        }
        FindAlgorithmTaskSettings cfg;
        
        // sequence
        QVariantMap qm = inputMessage.getData().toMap();
        SharedDbiDataHandler seqId = qm.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<SharedDbiDataHandler>();
        std::auto_ptr<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
        if (NULL == seqObj.get()) {
            return NULL;
        }
        DNASequence seq = seqObj->getWholeSequence();
        if(seq.isNull()) {
            return new FailTask(tr("Null sequence supplied to FindWorker: %1").arg(seq.getName()));
        }
        cfg.sequence = QByteArray(seq.constData(), seq.length());
        cfg.searchRegion.length = seq.length();
        
        // other parameters
        cfg.maxErr = actor->getParameter(ERR_ATTR)->getAttributeValue<int>(context);
        cfg.patternSettings = static_cast<FindAlgorithmPatternSettings> (actor->getParameter(ALGO_ATTR)->getAttributeValue<int>(context));
        cfg.useAmbiguousBases = actor->getParameter(AMBIGUOUS_ATTR)->getAttributeValue<bool>(context);

        resultName = actor->getParameter(NAME_ATTR)->getAttributeValue<QString>(context);
        
        // translations
        cfg.strand = getStrand(actor->getParameter(BaseAttributes::STRAND_ATTRIBUTE().getId())->getAttributeValue<QString>(context));
        if(cfg.strand != FindAlgorithmStrand_Direct /*&& seq.alphabet->getType() == DNAAlphabet_NUCL*/) {
            DNATranslation* compTT = AppContext::getDNATranslationRegistry()->
                lookupComplementTranslation(seq.alphabet);
            if (compTT != NULL) {
                cfg.complementTT = compTT ;
            } else {
                cfg.strand = FindAlgorithmStrand_Direct;
            }
        }
        if(actor->getParameter(AMINO_ATTR)->getAttributeValue<bool>(context)) {
            DNATranslationType tt = seq.alphabet->getType() == DNAAlphabet_NUCL ? DNATranslationType_NUCL_2_AMINO : DNATranslationType_RAW_2_AMINO;
            QList<DNATranslation*> TTs = AppContext::getDNATranslationRegistry()->lookupTranslation(seq.alphabet, tt);
            if (!TTs.isEmpty()) { //FIXME let user choose or use hints ?
                    cfg.proteinTT  = AppContext::getDNATranslationRegistry()->getStandardGeneticCodeTranslation(seq.alphabet);
            }
        }

        useNames = actor->getParameter(USE_NAMES_ATTR)->getAttributeValue<bool>(context);

        // for each pattern run find task
        QStringList ptrnStrs;
        if (qm.contains(BaseSlots::TEXT_SLOT().getId())) {
            ptrnStrs << qm.value(BaseSlots::TEXT_SLOT().getId()).toString();
        } else {
            ptrnStrs = actor->getParameter(PATTERN_ATTR)->getAttributeValue<QString>(context).split(PATTERN_DELIMITER, QString::SkipEmptyParts);
        }
        if(ptrnStrs.isEmpty() && namesPatterns.isEmpty()) {
            return new FailTask(tr("Empty pattern given"));
        }
        QList<Task*> subs;
        //pattern in parameters
        foreach(const QString & p, ptrnStrs) {
            assert(!p.isEmpty());
            FindAlgorithmTaskSettings config(cfg);
            config.pattern = p.toUpper().toLatin1();
            Task * findTask = new FindAlgorithmTask(config);
            patterns.insert(findTask, config.pattern);
            subs << findTask;
        }

        //patterns from file
        typedef QPair<QString, QString> NamePattern;
        foreach(const NamePattern& np, namesPatterns){
            FindAlgorithmTaskSettings config(cfg);
            config.pattern = np.second.toUpper().toLatin1();
            Task * findTask = new FindAlgorithmTask(config);
            filePatterns.insert(findTask, qMakePair(np.first,config.pattern));
            subs << findTask;
        }

        assert(!subs.isEmpty());
        
        MultiTask * multiFind = new MultiTask(tr("Find algorithm subtasks"), subs);
        connect(new TaskSignalMapper(multiFind), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
        return multiFind;
    } else if (input->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void FindWorker::sl_taskFinished(Task* t) {
    MultiTask * multiFind = qobject_cast<MultiTask*>(t);
    assert(multiFind != NULL);
    QList<Task*> subs = multiFind->getTasks();
    assert(!subs.isEmpty());
    QStringList ptrns;
    QList<FindAlgorithmResult> annData;
    QList<SharedAnnotationData> result;
    foreach(Task * sub, subs) {
        FindAlgorithmTask * findTask = qobject_cast<FindAlgorithmTask*>(sub);
        if(findTask != NULL){
            //parameters pattern
            if (!filePatterns.contains(sub)){
                annData << findTask->popResults();
                ptrns << patterns.value(findTask);
            }else{ //file pattern
                QString name = resultName;
                if (useNames){
                    QString pattName = filePatterns.value(findTask).first;
                    if(!pattName.isEmpty()){

                        QString newPatternName = pattName;
                        if (newPatternName.length() >= GBFeatureUtils::MAX_KEY_LEN){
                            newPatternName = pattName.left(GBFeatureUtils::MAX_KEY_LEN);
                        }

                        if (Annotation::isValidAnnotationName(newPatternName)){
                            name = newPatternName;
                        }
                    }
                }
                const QList<SharedAnnotationData>& curResult = FindAlgorithmResult::toTable(findTask->popResults(), name);
                result.append(curResult);
                if (output){
                    algoLog.info(tr("Found %1 matches of pattern '%2'").arg(curResult.size()).arg(QString(filePatterns.value(findTask).second)));
                }
            }
            
        }else{
            LoadPatternsFileTask * loadTask = qobject_cast<LoadPatternsFileTask*>(sub);
            if (loadTask != NULL){
                namesPatterns = loadTask->getNamesPatterns();
            }
            return;
        }
    }
    if(output) {
        const QList<SharedAnnotationData>& curResult = FindAlgorithmResult::toTable(annData, resultName);
        result.append(curResult);
        QVariant v = qVariantFromValue<QList<SharedAnnotationData> >(result);
        output->put(Message(BaseTypes::ANNOTATION_TABLE_TYPE(), v));
        algoLog.info(tr("Found %1 matches of pattern '%2'").arg(curResult.size()).arg(ptrns.join(PATTERN_DELIMITER)));
    }
}

void FindWorker::cleanup() {
}


/***************************
 * FindAllRegionsTask
 ***************************/
FindAllRegionsTask::FindAllRegionsTask(const FindAlgorithmTaskSettings& s, const QList<SharedAnnotationData>& l) :
Task(tr("FindAllRegionsTask"), TaskFlag_NoRun), cfg(s), regions(l) {}

void FindAllRegionsTask::prepare() {
    foreach(SharedAnnotationData sd, regions) {
        foreach(U2Region lr, sd->getRegions()) {
            cfg.searchRegion = lr;
            addSubTask(new FindAlgorithmTask(cfg));
        }
    }
}

QList<FindAlgorithmResult> FindAllRegionsTask::getResult() {
    QList<FindAlgorithmResult> lst;
    foreach(Task* t, getSubtasks()) {
        FindAlgorithmTask* ft = qobject_cast<FindAlgorithmTask*>(t);
        lst += ft->popResults();
    }
    return lst;
}

} //namespace LocalWorkflow
} //namespace U2
