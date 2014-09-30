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

#include "CreateAnnotationTask.h"

#include "LoadDocumentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/Timer.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

CreateAnnotationsTask::CreateAnnotationsTask( AnnotationTableObject *_ao, const QString &_g, const AnnotationData &_data )
    : Task( tr( "Create annotations" ), TaskFlags_FOSE_COSC ), aobj( _ao ), groupName( _g ), pos( 0 )
{
    aData << _data;
    aRef.objName = _ao->getGObjectName( );
    tpm = Progress_Manual;
}

CreateAnnotationsTask::CreateAnnotationsTask( AnnotationTableObject *_ao, const QString &_g, const QList<AnnotationData> &_data )
    : Task( tr( "Create annotations" ), TaskFlags_FOSE_COSC ), aobj( _ao ), groupName( _g ), pos( 0 )
{
    aData = _data;
    aRef.objName = _ao->getGObjectName( );
    tpm = Progress_Manual;
}

CreateAnnotationsTask::CreateAnnotationsTask( const GObjectReference &r, const QString &_g, const QList<AnnotationData> &_data )
    : Task( tr( "Create annotations" ), TaskFlags_FOSE_COSC ), aRef( r ), groupName( _g ), pos( 0 )
{
    aData << _data;
    GObject *ao = ( GObjectUtils::selectObjectByReference( aRef, UOF_LoadedAndUnloaded ) );
    if ( NULL != ao && ao->isUnloaded( ) ) {
        addSubTask( new LoadUnloadedDocumentTask( ao->getDocument( ) ) );
    }
    tpm = Progress_Manual;
}

void CreateAnnotationsTask::run() {
    U2OpStatusImpl os;
    AnnotationTableObject *parentObject = getGObject();
    const U2DataId rootFeatureId = parentObject->getRootFeatureId();
    const U2DbiRef dbiRef = parentObject->getEntityRef().dbiRef;
    const U2DataId groupId = AnnotationGroup(rootFeatureId, parentObject).getSubgroup(groupName, true).id;

    foreach (const AnnotationData &a, aData) {
        CHECK_OP(os, );
        importedFeatures << U2FeatureUtils::exportAnnotationDataToFeatures(a, rootFeatureId, groupId, dbiRef, os);
    }
}

Task::ReportResult CreateAnnotationsTask::report( ) {
    GTIMER( c1, t1, "CreateAnnotationsTask::report" );
    if ( hasError( ) || isCanceled( ) || importedFeatures.isEmpty( ) ) {
        return ReportResult_Finished;
    }
    AnnotationTableObject *ao = getGObject( );
    if ( NULL == ao ) {
        stateInfo.setError( tr( "Annotation object '%1' not found in active project: %2" ).arg( aRef.objName ).arg( aRef.docUrl ) );
    }

    if ( ao->isStateLocked( ) ) {
        stateInfo.setDescription( tr( "Waiting for object lock released" ) );
        return ReportResult_CallMeAgain;
    }
    stateInfo.setDescription( QString( ) );

    const int brk = qMin( 500, importedFeatures.size( ) - pos );
    GTIMER( c2, t2, "CreateAnnotationsTask::report [addAnnotations]" );
    AnnotationGroup( ao->getRootFeatureId( ), ao ).getSubgroup( groupName, false ).addFeatures( importedFeatures.mid( pos, brk ) );
    stateInfo.progress = 100 * ( pos + brk ) / importedFeatures.size( );
    if (pos + brk != importedFeatures.size()) {
        pos += brk;
        return ReportResult_CallMeAgain;
    }

    return ReportResult_Finished;
}

AnnotationTableObject * CreateAnnotationsTask::getGObject( ) const {
    AnnotationTableObject *result = NULL;
    if ( aRef.isValid( ) ) {
        SAFE_POINT( aobj.isNull( ), "Unexpected annotation table object content!", NULL );
        result = qobject_cast<AnnotationTableObject *>( GObjectUtils::selectObjectByReference( aRef, UOF_LoadedOnly ) );
    } else {
        result = aobj.data( );
    }
    return result;
}

int CreateAnnotationsTask::getAnnotationCount( ) const {
    return aData.size( );
}

} // namespace
