/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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


#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Log.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/GObject.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Counter.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/MultiTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "ShiftSequenceStartTask.h"

namespace U2 {

ShiftSequenceStartTask::ShiftSequenceStartTask( U2SequenceObject *_seqObj, int _shiftSize )

:Task(tr("ShiftSequenceStartTask"), TaskFlag_NoRun), seqObj(_seqObj),shiftSize(_shiftSize)
{
    GCOUNTER( cvar, tvar, "ShiftSequenceStartTask" );
}

Task::ReportResult ShiftSequenceStartTask::report(){

    if (!seqObj->isCircular() || shiftSize == 0) {
        return ReportResult_Finished;
    }

    CHECK(abs(shiftSize) < seqObj->getSequenceLength(), ReportResult_Finished);
    
    Document* curDoc = seqObj->getDocument();
    CHECK_EXT(!curDoc->isStateLocked(), setError(tr("Document is locked")), ReportResult_Finished);

    DNASequence dna = seqObj->getWholeSequence();
    int shiftPos = dna.seq.length() - shiftSize;
    dna.seq = dna.seq.mid(shiftPos, shiftSize) + dna.seq.mid(0, shiftPos);
    seqObj->setWholeSequence(dna);
    

    Project *p = AppContext::getProject();
    if (p != NULL){
        if (p->isStateLocked()){
            return ReportResult_CallMeAgain;
        }
        docs = p->getDocuments();
    }

    
    if (!docs.contains(curDoc)){
        docs.append(curDoc);
    }

    fixAnnotations();

    return ReportResult_Finished;
}


void ShiftSequenceStartTask::fixAnnotations( ) {
    foreach ( Document *d, docs ) {
        QList<GObject *> annotationTablesList = d->findGObjectByType( GObjectTypes::ANNOTATION_TABLE );
        foreach ( GObject *table, annotationTablesList ) {
            AnnotationTableObject *ato = qobject_cast<AnnotationTableObject *>( table );
            if ( ato->hasObjectRelation( seqObj, GObjectRelationRole::SEQUENCE ) ){
                foreach ( Annotation an, ato->getAnnotations( ) ) {
                    const U2Location& location = an.getLocation();
                    U2Location newLocation = shiftLocation(location, shiftSize);
                    an.setLocation(newLocation);
                    
                }
            }
        }
    }
}


U2Location ShiftSequenceStartTask::shiftLocation(const U2Location& location, int shiftSize) {
    // TODO: special handling for location intersecting 0
    U2Location newLocation(location);
    newLocation->regions.clear();
    
    foreach( const U2Region& r, location->regions) {
        U2Region newRegion(r.startPos + shiftSize, r.length );
        newLocation->regions.append(newRegion);
    }

    return newLocation;
}




}//ns
