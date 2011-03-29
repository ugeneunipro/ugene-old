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
        return ReportResult_CallMeAgain;
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
    ORFAlgorithmSettings cfg;
    
    /*
    QString locationStr = AppContext::getSettings()->getValue(NON_CUT_REGION, "").toString();
    U2Location location;
    Genbank::LocationParser::parseLocation(qPrintable(locationStr), locationStr.length(), location);
    if (!location->isEmpty()) {
        cfg.excludedRegions = location->regions;
    }
    */
    
    AnnotationTableObject* aObj = aa->getAnnotationObject();
    DNASequenceObject* dnaObj = aa->getSeqObject();
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
