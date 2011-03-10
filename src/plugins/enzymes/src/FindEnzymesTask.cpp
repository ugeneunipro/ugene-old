#include "FindEnzymesTask.h"
#include "EnzymesIO.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/Log.h>
#include <U2Core/Counter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabetUtils.h>
#include <U2Core/Settings.h>
#include <U2Formats/GenbankFeatures.h>
#include <U2Formats/GenbankLocationParser.h>

namespace U2 {

/* TRANSLATOR U2::FindEnzymesTask */

static Logger log(ULOG_ENZYME_PLUGIN);

//////////////////////////////////////////////////////////////////////////
// enzymes -> annotations

FindEnzymesToAnnotationsTask::FindEnzymesToAnnotationsTask(AnnotationTableObject* aobj, const DNASequence& seq, 
                                                           const QList<SEnzymeData>& _enzymes, const FindEnzymesTaskConfig& config)
: Task(tr("Find and store enzymes"), TaskFlags_FOSCOE), enzymes(_enzymes), aObj(aobj), cfg(config)
{
    GCOUNTER( cvar, tvar, "FindEnzymesToAnnotationsTask" );
    seqRange = U2Region(0, seq.length());
    fTask = new FindEnzymesTask(seq, seqRange, enzymes, cfg.maxResults, cfg.circular);
    addSubTask(fTask);
}

void FindEnzymesToAnnotationsTask::run() {

    if (isCanceled() || hasErrors()) {
        return;
    }

    if (aObj.isNull()) {
        stateInfo.setError(  tr("Annotation table does not exist") );
        return;
    }

    if (aObj->isStateLocked()) {
        stateInfo.setError(  tr("Annotation table is read-only") );
        return;
    }
    
    bool useSubgroups = enzymes.size() > 1 || cfg.groupName.isEmpty();
    bool useWholeSequenceRange = cfg.excludedRegions.isEmpty();
    foreach(const SEnzymeData& ed, enzymes) {
        QList<SharedAnnotationData> anns = fTask->getResultsAsAnnotations(ed->id);
        bool inRegion = false;
        if (!useWholeSequenceRange) {
            // filter 
            foreach (const SharedAnnotationData& data, anns) {
                const U2Region& annRegion = data->location->regions.first();
                if (annRegion.findOverlappingRegion(cfg.excludedRegions) != -1) {
                    inRegion = true;
                    break;
                }
            }
            if (inRegion) {
                continue;
            }
        }
        
        if(anns.size() >= cfg.minHitCount && anns.size() <= cfg.maxHitCount){
            QString group = useSubgroups ? cfg.groupName + "/" + ed->id : cfg.groupName;
            foreach(const SharedAnnotationData& ad, anns) {
                resultMap.insertMulti(group, ad);            
            }
        }
    }

   
    
}

Task::ReportResult FindEnzymesToAnnotationsTask::report()
{
    propagateSubtaskError();
    
    if (isCanceled() || hasErrors()) {
        return ReportResult_Finished;
    }

    const QSet<QString> groupNames = resultMap.keys().toSet();
    foreach (const QString& groupName, groupNames) {
        QList<Annotation*> annotations;
        QList<SharedAnnotationData> adata = resultMap.values(groupName);
        foreach (const SharedAnnotationData& ad, adata) {
            annotations.append(new Annotation(ad));
        }
        aObj->addAnnotations(annotations, groupName);
    }
 
    if (aObj->getAnnotations().isEmpty() && !cfg.autoUpdateTask) {
        // no results found -> delete empty annotation document
        Project* proj = AppContext::getProject();
        if (proj!=NULL) {
            Document* toDelete = NULL;
            QList<Document*> docs = proj->getDocuments();
            foreach (Document* doc, docs) {
                if (doc->getObjects().contains(aObj)) {
                    toDelete = doc;
                    break;
                }
            }
            if (toDelete != NULL) {
                proj->removeDocument(toDelete);
            }
        }
        aObj = NULL;
        stateInfo.setError("Enzymes selection is not found");
    }

    
    return ReportResult_Finished;
}


//////////////////////////////////////////////////////////////////////////
// find multiple enzymes task
FindEnzymesTask::FindEnzymesTask(const DNASequence& seq, const U2Region& region, const QList<SEnzymeData>& enzymes, int mr, bool _circular)
:Task(tr("Find Enzymes"), TaskFlags_NR_FOSCOE), maxResults(mr), circular(_circular)
{
    assert(seq.alphabet->isNucleic());
    seqlen = seq.length();
    //for every enzymes in selection create FindSingleEnzymeTask
    foreach(const SEnzymeData& e, enzymes) {
        addSubTask(new FindSingleEnzymeTask(seq, region, e, this, circular));
    }
}

void FindEnzymesTask::onResult(int pos, const SEnzymeData& enzyme) {
    QMutexLocker l(&resultsLock);
    if (results.size() > maxResults) {
        if (!isCanceled()) {
            stateInfo.setError(  tr("Number of results exceed %1, stopping").arg(maxResults) );
            cancel();
        }
        return;
    }
    results.append(FindEnzymesAlgResult(enzyme, pos));
}

QList<SharedAnnotationData> FindEnzymesTask::getResultsAsAnnotations(const QString& enzymeId) const {
    QList<SharedAnnotationData> res;
    
    QString cutStr;
    QString dbxrefStr;
    bool found = true;
    foreach(const FindEnzymesAlgResult& r, results) {
        if (r.enzyme->id != enzymeId) {
            continue;
        }
        found = true;
        if (!r.enzyme->accession.isEmpty()) {
            QString accession = r.enzyme->accession;
            if (accession.startsWith("RB")) {
                accession = accession.mid(2);
            }
            dbxrefStr = "REBASE:"+ accession;
        } else if (!r.enzyme->id.isEmpty()) {
            dbxrefStr = "REBASE:"+ r.enzyme->id;
        }
        if (r.enzyme->cutDirect != ENZYME_CUT_UNKNOWN) {
            cutStr = QString::number(r.enzyme->cutDirect);
            if (r.enzyme->cutComplement != ENZYME_CUT_UNKNOWN  && r.enzyme->cutComplement!=r.enzyme->cutDirect) {
                cutStr+="/"+QString::number(r.enzyme->cutComplement);
            }
        }
        break;
    }
    if (!found) {
        return res;
    }

    foreach(const FindEnzymesAlgResult& r, results) {
        if (r.enzyme->id == enzymeId) {
            if (circular && r.pos + r.enzyme->seq.size() > seqlen) {
                AnnotationData* ad1 = new AnnotationData();
                ad1->name = r.enzyme->id;
                if (seqlen < r.pos) {
                    delete ad1;
                    continue;
                }
                ad1->location->regions << U2Region(r.pos, seqlen - r.pos);
                if (!dbxrefStr.isEmpty()) {
                    ad1->qualifiers.append(U2Qualifier("db_xref", dbxrefStr));
                }
                if (!cutStr.isEmpty()) {
                    ad1->qualifiers.append(U2Qualifier(GBFeatureUtils::QUALIFIER_CUT, cutStr));
                }
                ad1->qualifiers.append(U2Qualifier("SPLIT", QString("%1").arg(r.enzyme->seq.size())));
                AnnotationData* ad2 = new AnnotationData();
                ad2->name = r.enzyme->id;
                if (r.enzyme->seq.size() < (seqlen - r.pos)) {
                    delete ad1;
                    delete ad2;
                    continue;
                }
                ad2->location->regions << U2Region(0, r.enzyme->seq.size() - (seqlen - r.pos));
                if (!dbxrefStr.isEmpty()) {
                    ad2->qualifiers.append(U2Qualifier("db_xref", dbxrefStr));
                }
                if (!cutStr.isEmpty()) {
                    ad2->qualifiers.append(U2Qualifier(GBFeatureUtils::QUALIFIER_CUT, cutStr));
                }
                ad2->qualifiers.append(U2Qualifier("SPLIT", QString("%1").arg(-r.enzyme->seq.size())));
                res.append(SharedAnnotationData(ad1));
                res.append(SharedAnnotationData(ad2));
            } else {
                AnnotationData* ad = new AnnotationData();
                ad->name = r.enzyme->id;
                ad->location->regions << U2Region(r.pos, r.enzyme->seq.size());
                if (!dbxrefStr.isEmpty()) {
                    ad->qualifiers.append(U2Qualifier("db_xref", dbxrefStr));
                }
                if (!cutStr.isEmpty()) {
                    ad->qualifiers.append(U2Qualifier(GBFeatureUtils::QUALIFIER_CUT, cutStr));
                }
                res.append(SharedAnnotationData(ad));
            }
        }
    }
    return res;
}

Task::ReportResult FindEnzymesTask::report() {
    if (!hasErrors() && !isCanceled()) {
        log.info(tr("Found %1 restriction sites").arg(results.count()));
    }
    return ReportResult_Finished;
}

void FindEnzymesTask::cleanup() {
    results.clear();
}

//////////////////////////////////////////////////////////////////////////
// find single enzyme task
FindSingleEnzymeTask::FindSingleEnzymeTask(const DNASequence& _seq, const U2Region& region, const SEnzymeData& _enzyme, 
                                           FindEnzymesAlgListener* l, bool _circular, int mr)
: Task(tr("Find enzyme '%1'").arg(_enzyme->id), TaskFlag_NoRun), 
dna(_seq), region(region), enzyme(_enzyme), maxResults(mr), resultListener(l), circular(_circular)
{
    assert(dna.alphabet->isNucleic());
    if (resultListener == NULL) {
        resultListener = this;
    }
    if (region.length != dna.length()) {
        circular = false;
    }
    SequenceWalkerConfig swc;
    
    swc.seq = dna.seq.constData() + region.startPos;
    swc.seqSize = region.length;
    swc.chunkSize = region.length;
    
    addSubTask(new SequenceWalkerTask(swc, this, tr("Find enzyme '%1' parallel").arg(enzyme->id)));
}

void FindSingleEnzymeTask::onResult(int pos, const SEnzymeData& enzyme) {
    if (circular && pos >= region.length) {
        return;
    }
    QMutexLocker l(&resultsLock);
    if (results.size() > maxResults) {
        if (!isCanceled()) {
            stateInfo.setError(  FindEnzymesTask::tr("Number of results exceed %1, stopping").arg(maxResults) );
            cancel();
        }
        return;
    }
    results.append(FindEnzymesAlgResult(enzyme, pos));
}

void FindSingleEnzymeTask::onRegion(SequenceWalkerSubtask* t, TaskStateInfo& ti) {
    if (enzyme->seq.isEmpty()) {
        return;
    }
    if (dna.length() < enzyme->seq.length()) {
        return;
    }
    if (!enzyme->alphabet->isNucleic()) {
        log.info(tr("Non-nucleic enzyme alphabet: %1, enzyme: %2, skipping..").arg(enzyme->alphabet->getId()).arg(enzyme->id));
        return;
    }
    bool useExtendedComparator = enzyme->alphabet->getId() == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED() 
                                || dna.alphabet->getId() == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED();
  
    const SequenceWalkerConfig& c = t->getGlobalConfig();
    if (useExtendedComparator) {
        FindEnzymesAlgorithm<ExtendedDNAlphabetComparator> algo;
        algo.run(dna, c.range, enzyme, resultListener, ti);
    } else {
        FindEnzymesAlgorithm<ExactDNAAlphabetComparatorNX> algo;
        algo.run(dna, c.range, enzyme, resultListener, ti);
    }
}

void FindSingleEnzymeTask::cleanup() {
    results.clear();
}


FindEnzymesAutoAnnotationUpdater::FindEnzymesAutoAnnotationUpdater()
: AutoAnnotationsUpdater(tr("Restriction Sites"), ANNOTATION_GROUP_ENZYME)
{

}

Task* FindEnzymesAutoAnnotationUpdater::createAutoAnnotationsUpdateTask( const AutoAnnotationObject* aa )
{
    const QList<SEnzymeData> enzymeList =  EnzymesIO::getDefaultEnzymesList();
    QString selStr = AppContext::getSettings()->getValue(LAST_SELECTION_KEY).toString();
    QStringList lastSelection = selStr.split(SEP);

    QList<SEnzymeData> selectedEnzymes;
    foreach (const QString id, lastSelection) {
        foreach (const SEnzymeData& enzyme, enzymeList) {
            if (id == enzyme->id) {
                selectedEnzymes.append(enzyme);
            }
        }
    }

    FindEnzymesTaskConfig cfg;
    U2Region range = aa->getSeqObject()->getSequenceRange();
    cfg.circular = aa->getSeqObject()->isCircular();
    cfg.groupName = getGroupName();
    cfg.autoUpdateTask = true;
    cfg.minHitCount = AppContext::getSettings()->getValue(MIN_HIT_VALUE, 1).toInt();
    cfg.maxHitCount = AppContext::getSettings()->getValue(MAX_HIT_VALUE, INT_MAX).toInt();
    cfg.maxResults = AppContext::getSettings()->getValue(MAX_RESULTS, 500000).toInt();

    QString locationStr = AppContext::getSettings()->getValue(NON_CUT_REGION, "").toString();
    U2Location location;
    Genbank::LocationParser::parseLocation(qPrintable(locationStr), locationStr.length(), location);
    if (!location->isEmpty()) {
        cfg.excludedRegions = location->regions;
    }

    const DNASequence& dna = aa->getSeqObject()->getDNASequence();
    
    if (!dna.alphabet->isNucleic()) {
        return NULL;
    }

    AnnotationTableObject* aObj = aa->getAnnotationObject();
    Task* task = new FindEnzymesToAnnotationsTask(aObj, dna, selectedEnzymes, cfg );

    return task;
}

bool FindEnzymesAutoAnnotationUpdater::checkConstraints( const AutoAnnotationConstraints& constraints )
{
    if (constraints.alphabet == NULL) {
        return false;
    }

    return constraints.alphabet->isNucleic();
}




}//namespace
