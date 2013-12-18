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

#include <U2Core/DNASequenceObject.h>
#include <U2Core/FeaturesTableObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNASequenceSelection.h>

#include "ReverseSequenceTask.h"

namespace U2 {

ReverseSequenceTask::ReverseSequenceTask( U2SequenceObject *dObj,
    const QList<FeaturesTableObject *> &annotations, DNASequenceSelection *s, DNATranslation *transl )
    : Task( "ReverseSequenceTask", TaskFlags_NR_FOSCOE ), seqObj( dObj ), aObjs( annotations ),
    selection( s ), complTr( transl )
{

}

Task::ReportResult ReverseSequenceTask::report( ) {
    DNASequence sequence = seqObj->getWholeSequence( );
    if ( sequence.seq.isEmpty( ) ) {
        return ReportResult_Finished;
    }

    char *data = sequence.seq.data( );
    const int len = sequence.length( );
    if ( NULL != complTr ) {
        complTr->translate( data, len );
    }
    TextUtils::reverse( data, len );
    seqObj->setWholeSequence( sequence );

    // mirror selection
    if ( NULL != selection ) {
        QVector<U2Region> regions = selection->getSelectedRegions( );
        U2Region::mirror( len, regions );
        U2Region::reverse( regions );
        selection->setSelectedRegions( regions );
    }
    
    // fix annotation locations
    foreach ( FeaturesTableObject *aObj, aObjs ) {
        foreach ( __Annotation a, aObj->getAnnotations( ) ) {
            if ( NULL != complTr ) {
                U2Strand strand = a.getStrand( );
                a.setStrand( strand == U2Strand::Direct
                    ? U2Strand::Complementary : U2Strand::Direct );
            }
            U2Location location = a.getLocation( );
            U2Region::mirror( len, location->regions );
            U2Region::reverse( location->regions );
            a.setLocation( location );
        }
    }

    return ReportResult_Finished;
}


}//namespace
