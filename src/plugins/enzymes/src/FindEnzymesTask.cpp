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

#include "FindEnzymesTask.h"
#include "EnzymesIO.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/Log.h>
#include <U2Core/Counter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/Settings.h>
#include <U2Formats/GenbankFeatures.h>
#include <U2Formats/GenbankLocationParser.h>

namespace U2 {

/* TRANSLATOR U2::FindEnzymesTask */

//////////////////////////////////////////////////////////////////////////
// enzymes -> annotations

FindEnzymesToAnnotationsTask::FindEnzymesToAnnotationsTask(AnnotationTableObject* aobj, const DNASequence& seq, 
                                                           const QList<SEnzymeData>& _enzymes, const FindEnzymesTaskConfig& config)
: Task(tr("Find and store enzymes"), TaskFlags_FOSCOE), dnaSeq(seq), enzymes(_enzymes), aObj(aobj), cfg(config)
{
    GCOUNTER( cvar, tvar, "FindEnzymesToAnnotationsTask" );
    seqRange = U2Region(0, seq.length());
   
}


void FindEnzymesToAnnotationsTask::prepare()
{
     if (enzymes.count() == 0 ) {
        stateInfo.setError(tr("No enzymes selected."));
        return;
    }
    fTask = new FindEnzymesTask(dnaSeq, seqRange, enzymes, cfg.maxResults, cfg.circular);
    addSubTask(fTask);
}


void FindEnzymesToAnnotationsTask::run() {

    if (isCanceled() || hasError()) {
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
    if (isCanceled() || hasError()) {
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
 
    if (aObj->getAnnotations().isEmpty() && !cfg.isAutoAnnotationUpdateTask) {
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

void FindEnzymesTask::onResult(int pos, const SEnzymeData& enzyme, const U2Strand& strand) {
    QMutexLocker l(&resultsLock);
    if (results.size() > maxResults) {
        if (!isCanceled()) {
            stateInfo.setError(  tr("Number of results exceed %1, stopping").arg(maxResults) );
            cancel();
        }
        return;
    }
    results.append(FindEnzymesAlgResult(enzyme, pos, strand));
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
                if (seqlen < r.pos) {
                    continue;
                }
                AnnotationData* ad = new AnnotationData();
                ad->name = r.enzyme->id;
                ad->location->regions << U2Region(r.pos, seqlen - r.pos);
                ad->location->regions << U2Region(0, r.enzyme->seq.size() - (seqlen - r.pos));
                ad->setStrand(r.strand);
                if (!dbxrefStr.isEmpty()) {
                    ad->qualifiers.append(U2Qualifier("db_xref", dbxrefStr));
                }
                if (!cutStr.isEmpty()) {
                    ad->qualifiers.append(U2Qualifier(GBFeatureUtils::QUALIFIER_CUT, cutStr));
                }
                res.append(SharedAnnotationData(ad));
            } else {
                AnnotationData* ad = new AnnotationData();
                ad->name = r.enzyme->id;
                ad->location->regions << U2Region(r.pos, r.enzyme->seq.size());
                ad->setStrand(r.strand);
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
    if (!hasError() && !isCanceled()) {
        algoLog.info(tr("Found %1 restriction sites").arg(results.count()));
    }
    return ReportResult_Finished;
}

void FindEnzymesTask::cleanup() {
    if (hasError()) {
        results.clear();
    }
}

//////////////////////////////////////////////////////////////////////////
// find single enzyme task
FindSingleEnzymeTask::FindSingleEnzymeTask(const DNASequence& _seq, const U2Region& region, const SEnzymeData& _enzyme, 
                                           FindEnzymesAlgListener* l, bool _circular, int mr)
: Task(tr("Find enzyme '%1'").arg(_enzyme->id), TaskFlag_NoRun), 
dnaSeq(_seq), region(region), enzyme(_enzyme), maxResults(mr), resultListener(l), circular(_circular)
{
    assert(dnaSeq.alphabet->isNucleic());
    if (resultListener == NULL) {
        resultListener = this;
    }
    if (region.length != dnaSeq.length()) {
        circular = false;
    }
    SequenceWalkerConfig swc;
    
    swc.seq = dnaSeq.seq.constData() + region.startPos;
    swc.seqSize = region.length;
    swc.chunkSize = region.length;
    
    addSubTask(new SequenceWalkerTask(swc, this, tr("Find enzyme '%1' parallel").arg(enzyme->id)));
}

void FindSingleEnzymeTask::onResult(int pos, const SEnzymeData& enzyme, const U2Strand& strand) {
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
    results.append(FindEnzymesAlgResult(enzyme, pos, strand));
}

void FindSingleEnzymeTask::onRegion(SequenceWalkerSubtask* t, TaskStateInfo& ti) {
    if (enzyme->seq.isEmpty()) {
        return;
    }
    if (dnaSeq.length() < enzyme->seq.length()) {
        return;
    }
    if (!enzyme->alphabet->isNucleic()) {
        algoLog.info(tr("Non-nucleic enzyme alphabet: %1, enzyme: %2, skipping..").arg(enzyme->alphabet->getId()).arg(enzyme->id));
        return;
    }
    bool useExtendedComparator = enzyme->alphabet->getId() == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED() 
                                || dnaSeq.alphabet->getId() == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED() 
                                || dnaSeq.alphabet->getId() == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT()
                                || dnaSeq.alphabet->getId() == BaseDNAAlphabetIds::NUCL_RNA_EXTENDED();
  
    const SequenceWalkerConfig& c = t->getGlobalConfig();

    // Note that enzymes algorithm filters N symbols in sequence by itself
    if (useExtendedComparator) {
        FindEnzymesAlgorithm<ExtendedDNAlphabetComparator> algo;
        algo.run(dnaSeq, c.range, enzyme, resultListener, ti);
    } else {
        FindEnzymesAlgorithm<ExactDNAAlphabetComparatorN1M_N2M> algo;
        algo.run(dnaSeq, c.range, enzyme, resultListener, ti);
    }
}

void FindSingleEnzymeTask::cleanup() {
    results.clear();
}

//////////////////////////////////////////////////////////////////////////
// find enzymes auto annotation updater

FindEnzymesAutoAnnotationUpdater::FindEnzymesAutoAnnotationUpdater()
: AutoAnnotationsUpdater(tr("Restriction Sites"), ANNOTATION_GROUP_ENZYME)
{

}

Task* FindEnzymesAutoAnnotationUpdater::createAutoAnnotationsUpdateTask( const AutoAnnotationObject* aa )
{
    const QList<SEnzymeData> enzymeList =  EnzymesIO::getDefaultEnzymesList();
    QString selStr = AppContext::getSettings()->getValue(EnzymeSettings::LAST_SELECTION).toString();
    if (selStr.isEmpty()) {
        selStr = EnzymeSettings::COMMON_ENZYMES;
    }
    
    QStringList lastSelection = selStr.split(ENZYME_LIST_SEPARATOR);
    QList<SEnzymeData> selectedEnzymes;
    foreach (const QString id, lastSelection) {
        foreach (const SEnzymeData& enzyme, enzymeList) {
            if (id == enzyme->id) {
                selectedEnzymes.append(enzyme);
            }
        }
    }

    FindEnzymesTaskConfig cfg;
    cfg.circular = aa->getSeqObject()->isCircular();
    cfg.groupName = getGroupName();
    cfg.isAutoAnnotationUpdateTask = true;
    cfg.minHitCount = AppContext::getSettings()->getValue(EnzymeSettings::MIN_HIT_VALUE, 1).toInt();
    cfg.maxHitCount = AppContext::getSettings()->getValue(EnzymeSettings::MAX_HIT_VALUE, INT_MAX).toInt();
    cfg.maxResults = AppContext::getSettings()->getValue(EnzymeSettings::MAX_RESULTS, 500000).toInt();

    QString locationStr = AppContext::getSettings()->getValue(EnzymeSettings::NON_CUT_REGION, "").toString();
    
    QVector<U2Region> excludedRegions = 
        AppContext::getSettings()->getValue(EnzymeSettings::NON_CUT_REGION, QVariant::fromValue(QVector<U2Region>()) ).value< QVector<U2Region> >();
    
    if (!excludedRegions.isEmpty()) {
        cfg.excludedRegions = excludedRegions;
    }
    
    AnnotationTableObject* aObj = aa->getAnnotationObject();
    DNASequence dna = aa->getSeqObject()->getWholeSequence();
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
