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


#include <U2Core/DNASequence.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/Settings.h>
#include <U2Core/DNAAlphabet.h>

#include "ORFMarkerTask.h"

namespace U2 {

const QString ORFSettingsKeys::STRAND("orf_finder/strand");
const QString ORFSettingsKeys::ALLOW_ALT_START("orf_finder/allow_alt_start");
const QString ORFSettingsKeys::ALLOW_OVERLAP("orf_finder/allow_overlap");
const QString ORFSettingsKeys::AMINO_TRANSL("orf_finder/amino_transl");
const QString ORFSettingsKeys::MIN_LEN("orf_finder/min_length");
const QString ORFSettingsKeys::MUST_FIT("orf_finder/must_fit");
const QString ORFSettingsKeys::MUST_INIT("orf_finder/must_init");
const QString ORFSettingsKeys::SEARCH_REGION("orf_finder/region");
const QString ORFSettingsKeys::INCLUDE_STOP_CODON("orf_finder/incldue_stop_codon");

void ORFSettingsKeys::save(const ORFAlgorithmSettings& cfg, Settings* s) {
    s->setValue(ORFSettingsKeys::AMINO_TRANSL, cfg.proteinTT->getTranslationId());
    s->setValue(ORFSettingsKeys::MUST_FIT, cfg.mustFit);
    s->setValue(ORFSettingsKeys::MUST_INIT, cfg.mustInit);
    s->setValue(ORFSettingsKeys::ALLOW_ALT_START, cfg.allowAltStart);
    s->setValue(ORFSettingsKeys::ALLOW_OVERLAP, cfg.allowOverlap);
    s->setValue(ORFSettingsKeys::MIN_LEN, cfg.minLen);
    s->setValue(ORFSettingsKeys::SEARCH_REGION, QVariant::fromValue(cfg.searchRegion));
    s->setValue(ORFSettingsKeys::STRAND, ORFAlgorithmSettings::getStrandStringId(cfg.strand));
	s->setValue(ORFSettingsKeys::INCLUDE_STOP_CODON, cfg.includeStopCodon);
}

void ORFSettingsKeys::read(ORFAlgorithmSettings& cfg, const Settings* s) {
    cfg.mustFit = s->getValue(ORFSettingsKeys::MUST_FIT, false).toBool();
    cfg.mustInit = s->getValue(ORFSettingsKeys::MUST_INIT, true).toBool();
    cfg.allowAltStart = s->getValue(ORFSettingsKeys::ALLOW_ALT_START, false).toBool();
    cfg.allowOverlap = s->getValue(ORFSettingsKeys::ALLOW_OVERLAP, false).toBool();
    cfg.minLen = s->getValue(ORFSettingsKeys::MIN_LEN, 100).toInt();

    QString strandId = s->getValue(ORFSettingsKeys::STRAND, ORFAlgorithmSettings::STRAND_BOTH).toString();
    cfg.strand = ORFAlgorithmSettings::getStrandByStringId(strandId);

    QString translId = s->getValue(ORFSettingsKeys::AMINO_TRANSL, "").toString();
    if (!translId.isEmpty()) {
        cfg.proteinTT = AppContext::getDNATranslationRegistry()->lookupTranslation(translId);
    }
	
	cfg.includeStopCodon = s->getValue(ORFSettingsKeys::INCLUDE_STOP_CODON, false).toBool();
    cfg.searchRegion = s->getValue(ORFSettingsKeys::SEARCH_REGION).value<U2Region>();
}


//////////////////////////////////////////////////////////////////////////
// find ORFS and save 2 annotations task

FindORFsToAnnotationsTask::FindORFsToAnnotationsTask( AnnotationTableObject* aobj, const DNASequence& seq, const ORFAlgorithmSettings& settings )
  :  Task(tr("Find ORFs and save to annotations"), TaskFlags_FOSCOE), aObj(aobj), cfg(settings), dna(seq)
{
    fTask = new ORFFindTask(cfg, dna.seq);
    addSubTask(fTask);
}


void U2::FindORFsToAnnotationsTask::run()
{   
    QList<ORFFindResult> results =  fTask->popResults();
    
    foreach( const ORFFindResult& res, results) {
        aData.append(res.toAnnotation(ORFAlgorithmSettings::ANNOTATION_GROUP_NAME));
    }
}

Task::ReportResult U2::FindORFsToAnnotationsTask::report()
{
    
    if (isCanceled() || hasError()) {
        return ReportResult_Finished;
    }
    
    if (aObj->isStateLocked()) {
        setError(tr("Annotation obj %1 is locked for modifications").arg(aObj->getGObjectName()));
        return ReportResult_Finished;
    }
    
    QList<Annotation*> annotations;
    foreach (const SharedAnnotationData& data, aData) {
        annotations.append(new Annotation(data));
    }

    aObj->addAnnotations(annotations, ORFAlgorithmSettings::ANNOTATION_GROUP_NAME);
    

    return ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////
// ORFs auto annotation updater

ORFAutoAnnotationsUpdater::ORFAutoAnnotationsUpdater()
: AutoAnnotationsUpdater(tr("ORFs"), ORFAlgorithmSettings::ANNOTATION_GROUP_NAME)
{
    
}

Task* ORFAutoAnnotationsUpdater::createAutoAnnotationsUpdateTask( const AutoAnnotationObject* aa )
{
    AnnotationTableObject* aObj = aa->getAnnotationObject();
    U2SequenceObject* dnaObj = aa->getSeqObject();
    
    ORFAlgorithmSettings cfg;
    ORFSettingsKeys::read(cfg, AppContext::getSettings());
    
    cfg.complementTT = GObjectUtils::findComplementTT(dnaObj->getAlphabet());
    if (cfg.proteinTT == NULL) {
        cfg.proteinTT = GObjectUtils::findAminoTT(dnaObj,false);
    }
    qint64 seqLen = dnaObj->getSequenceLength();
    if (cfg.searchRegion.isEmpty() || cfg.searchRegion.endPos() >= seqLen + 1 ) {
        cfg.searchRegion = U2Region(0, dnaObj->getSequenceLength());
    }
    
    Task* task = new FindORFsToAnnotationsTask(aObj, dnaObj->getWholeSequence(), cfg );

    return task;
    
}

bool ORFAutoAnnotationsUpdater::checkConstraints( const AutoAnnotationConstraints& constraints ) {
    if (constraints.alphabet == NULL) {
        return false;
    }
    return constraints.alphabet->isNucleic();
}


} //~namespace
