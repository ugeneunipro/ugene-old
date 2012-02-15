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

#include <cassert>

#include <QtCore/QMutexLocker>

#include "uHMMSearchTaskLocalStorage.h"

namespace U2 {

QHash< qint64, UHMM3SearchTaskLocalData* > UHMM3SearchTaskLocalStorage::data;
QThreadStorage< ContextId* > UHMM3SearchTaskLocalStorage::tls;
QMutex UHMM3SearchTaskLocalStorage::mutex;
const UHMM3SearchTaskLocalData UHMM3SearchTaskLocalStorage::defaultData;

const UHMM3SearchTaskLocalData* UHMM3SearchTaskLocalStorage::current() {
    ContextId* idc = tls.localData();
    if( NULL != idc ) {
        QMutexLocker locker( &mutex );
        UHMM3SearchTaskLocalData* res = data.value( idc->id );
        assert( NULL != res );
        return res;
    } else {
        return &defaultData;
    }
}

UHMM3SearchTaskLocalData* UHMM3SearchTaskLocalStorage::createTaskContext( qint64 ctxId ) {
    QMutexLocker locker( &mutex );
    assert( !data.contains( ctxId ) );
    UHMM3SearchTaskLocalData* ctx = new UHMM3SearchTaskLocalData();
    data[ctxId] = ctx;
    
    assert(!tls.hasLocalData());
    ContextId* idc = new ContextId( ctxId );
    tls.setLocalData( idc );
    
    return ctx;
}

void UHMM3SearchTaskLocalStorage::freeTaskContext( qint64 ctxId ) {
    QMutexLocker locker( &mutex );
    
    UHMM3SearchTaskLocalData* lData = data.value( ctxId );
    assert( NULL != lData );
    
    int howMany = data.remove( ctxId );
    Q_UNUSED( howMany );
    assert( 1 == howMany );
    
    delete lData;
}

} // U2
