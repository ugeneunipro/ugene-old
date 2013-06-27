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

#include <U2Core/FailTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AppContext.h>

#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>

#include "RequestForSnpTask.h"
#include "BaseRequestForSnpWorker.h"

namespace U2 {

namespace LocalWorkflow {

BaseRequestForSnpWorker::BaseRequestForSnpWorker( Actor *p )
    : BaseWorker( p ), inChannel( NULL ), outChannel( NULL )
{

}

void BaseRequestForSnpWorker::init( )
{
    inChannel = ports.value( BasePorts::IN_VARIATION_TRACK_PORT_ID( ) );
    outChannel = ports.value( BasePorts::OUT_VARIATION_TRACK_PORT_ID( ) );
}

Task* BaseRequestForSnpWorker::tick( )
{
    U2OpStatus2Log os;
    if ( inChannel->hasMessage( ) ) {
        Message m = getMessageAndSetupScriptValues( inChannel );
        QVariantMap data = m.getData( ).toMap( );

        QVariant inVar;
        if ( !data.contains(BaseSlots::VARIATION_TRACK_SLOT( ).getId( ) ) ) {
            os.setError( "Variations slot is empty" );
            return new FailTask( os.getError( ) );
        }

        QScopedPointer<VariantTrackObject> trackObj( NULL );
        {
            SharedDbiDataHandler objId = data.value( BaseSlots::VARIATION_TRACK_SLOT( ).getId( ) )
                .value<SharedDbiDataHandler>( );
            trackObj.reset( StorageUtils::getVariantTrackObject( context->getDataStorage( ), objId ) );
            SAFE_POINT( NULL != trackObj.data( ), tr( "Can't get track object" ), NULL );

        }
        U2DbiRef dbiRef = trackObj->getEntityRef( ).dbiRef;
        U2VariantTrack track = trackObj->getVariantTrack( os );
        if ( os.hasError( ) ) {
            return new FailTask( os.getError( ) );
        }
        // TODO: obtain sequences from DB
        Task* t = new RequestForSnpTask( getRequestingScriptPath( ), getInputDataForRequest( ) );
        connect( t, SIGNAL( si_stateChanged( ) ), SLOT( sl_taskFinished( ) ) );
        return t;
    }
    if ( inChannel->isEnded( ) ) {
        setDone( );
        outChannel->setEnded( );
    }
    return NULL;
}

void BaseRequestForSnpWorker::sl_taskFinished( )
{
    RequestForSnpTask *t = dynamic_cast<RequestForSnpTask *>( sender( ) );
    SAFE_POINT( NULL != t, "Invalid task is encountered", );
    if ( !t->isFinished( ) || t->hasError( ) ) {
        return;
    }
    // TODO: get results from task, put them to DB
    // t->getResult( );
    outChannel->put( Message::getEmptyMapMessage( ) );
    if ( inChannel->isEnded( ) && !inChannel->hasMessage( ) ) {
        setDone( );
        outChannel->setEnded( );
    }
}

QString BaseRequestForSnpWorker::getRequestingScriptPath( ) const
{
    QString result( AppContext::getWorkingDirectoryPath( )
        + "/../../data/snp_scripts/" + getRequestingScriptName( ) );
    QFileInfo info( result );
    return info.absoluteFilePath( );
}


} // namespace LocalWorkflow

} // namespace U2