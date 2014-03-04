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

#include <U2Algorithm/SplicedAlignmentTask.h>
#include <U2Algorithm/SplicedAlignmentTaskRegistry.h>
#include <U2Core/LoadRemoteDocumentTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AppContext.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/AnnotationTableObject.h>

#include "FindExonRegionsTask.h"

namespace U2 {

extern Logger log( "Span Exon/Intron Regions" );

FindExonRegionsTask::FindExonRegionsTask( U2SequenceObject *dObj, const QString &rnaSeqId )
    : Task( "FindExonRegionsTask", TaskFlags_NR_FOSCOE ), dnaObj( dObj )
{
    if ( !rnaSeqId.isEmpty( ) ) {
        loadDocumentTask = new LoadRemoteDocumentTask( rnaSeqId, "genbank" );
    } else {
        loadDocumentTask = NULL;
    }

    alignmentTask = NULL;
}

void FindExonRegionsTask::prepare( ) {
    if ( loadDocumentTask ) {
        addSubTask( loadDocumentTask );
    }
}

QList<Task *> FindExonRegionsTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> res;

    if ( !subTask->isFinished( ) ) {
        return res;
    }

    if ( subTask == loadDocumentTask ) {
        Document *doc = loadDocumentTask->getDocument( );
        QList<GObject *> objects = doc->findGObjectByType( GObjectTypes::SEQUENCE );
        if ( objects.isEmpty( ) ) {
            setError(tr("Failed to load RNA sequence from %2").arg(doc->getName()));
            return res;
        }

        U2SequenceObject *rnaSeqObj = qobject_cast<U2SequenceObject *>( objects.first( ) );
        SAFE_POINT( NULL != rnaSeqObj, tr( "Failed to load RNA sequence" ), res );

        SplicedAlignmentTaskConfig cfg( rnaSeqObj, dnaObj );

        SplicedAlignmentTaskRegistry *sr = AppContext::getSplicedAlignmentTaskRegistry( );
        QStringList algList = sr->getAlgNameList( );

        if ( algList.isEmpty( ) ) {
            log.trace( tr( "No algorithm found to align cDNA sequence" ) );
            return res;
        }

        alignmentTask = sr->getAlgorithm( algList.first( ) )->createTaskInstance( cfg ) ;
        res.append( alignmentTask );
    } else if (subTask == alignmentTask) {
        QList<AnnotationData> results = alignmentTask->getAlignmentResult( );
        foreach ( const AnnotationData &ann, results ) {
            exonRegions.append( ann.location->regions.toList( ) );
        }
    }

    return res;
}

Task::ReportResult FindExonRegionsTask::report( ) {
    if ( NULL != loadDocumentTask ) {
        QList<GObject *> relAnns = GObjectUtils::findObjectsRelatedToObjectByRole( dnaObj,
            GObjectTypes::ANNOTATION_TABLE, GObjectRelationRole::SEQUENCE,
            dnaObj->getDocument( )->getObjects( ), UOF_LoadedOnly );
        AnnotationTableObject *att = relAnns.isEmpty( ) ? NULL
            : qobject_cast<AnnotationTableObject *>( relAnns.first( ) );
        
        if ( NULL != att ) {
            setError( tr( "Failed to search for exon annotations. "
                "The sequence %1 doesn't have any related annotations." )
                .arg( dnaObj->getSequenceName( ) ) );
            return ReportResult_Finished;
        }

        const QList<Annotation> anns = att->getAnnotations( );

        foreach ( const Annotation &ann, anns ) {
            if ( ann.getName( ) == "exon" ) {
                foreach ( const U2Region &r, ann.getRegions( ) ) {
                    exonRegions.append( r );
                }
            }
        }
        qSort( exonRegions );
    }

    return ReportResult_Finished;
}

} // namespace U2
