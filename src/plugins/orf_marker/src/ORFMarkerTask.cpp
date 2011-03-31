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

#include "ORFMarkerTask.h"

namespace U2 {

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
    
    if (isCanceled() || hasErrors()) {
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
    DNASequenceObject* dnaObj = aa->getSeqObject();
    DNAAlphabet* al = dnaObj->getAlphabet();

    assert(al != NULL);

    ORFAlgorithmSettings cfg;
    
    cfg.complementTT = GObjectUtils::findComplementTT(dnaObj);
    QString translId = AppContext::getSettings()->getValue(ORFSettingsKeys::AMINO_TRANSL, "").toString();
    if (translId.isEmpty()) {
        cfg.proteinTT = GObjectUtils::findAminoTT(dnaObj,false);
    } else {
        cfg.proteinTT = AppContext::getDNATranslationRegistry()->lookupTranslation(translId);
    }
    cfg.minLen = AppContext::getSettings()->getValue(ORFSettingsKeys::MIN_LEN, 100).toInt();
    cfg.searchRegion = AppContext::getSettings()->getValue(ORFSettingsKeys::SEARCH_REGION, 
        QVariant::fromValue(U2Region(0, dnaObj->getSequenceLen()))).value<U2Region>();
    
    Task* task = new FindORFsToAnnotationsTask(aObj, dnaObj->getDNASequence(), cfg );

    return task;
    
}

bool ORFAutoAnnotationsUpdater::checkConstraints( const AutoAnnotationConstraints& constraints )
{
    if (constraints.alphabet == NULL) {
        return false;
    }

    return constraints.alphabet->isNucleic();
}


} //~namespace
