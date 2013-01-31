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

#include "CopyDataTask.h"

#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>

#include <QtCore/QString>
#include <QtCore/QByteArray>

#include <memory>

namespace U2 {

CopyDataTask::CopyDataTask( IOAdapterFactory * _ioFrom, const GUrl& _urlFrom, 
                           IOAdapterFactory * _ioTo, const GUrl& _urlTo ) :
Task(tr("Copy Data Task"), TaskFlag_None), ioFrom(_ioFrom), ioTo(_ioTo),
urlFrom(_urlFrom), urlTo(_urlTo)
{
    assert(ioFrom != NULL && ioTo != NULL);
    tpm = Progress_Manual;
}

void CopyDataTask::run() {
    std::auto_ptr<IOAdapter> from( ioFrom->createIOAdapter() );
    std::auto_ptr<IOAdapter> where( ioTo->createIOAdapter() );
    from->open( urlFrom, IOAdapterMode_Read );
    if (!from->isOpen()) {
        stateInfo.setError(L10N::errorOpeningFileRead(urlFrom));
        return;
    }
    
    qint64 count = 0;
    qint64 count_w = 0;
    QByteArray buff( BUFFSIZE, 0 );
    
    count = from->readBlock( buff.data(), BUFFSIZE );
    if (count == 0) {
        stateInfo.setError(tr("Cannot get data from: '%1'").arg(urlFrom.getURLString()));
        return;
    }
    
    if (!where->open( urlTo, IOAdapterMode_Write )) {
        stateInfo.setError(L10N::errorOpeningFileWrite(urlTo));
        return;
    }
    
    while( count > 0 ) {
        count_w = where->writeBlock( buff.data(), count );
        if( stateInfo.cancelFlag ) {
            break;
        }
        stateInfo.progress = from->getProgress();
        count = from->readBlock( buff.data(), BUFFSIZE );
    }
    if( count < 0 || count_w < 0 ) {
        if (!stateInfo.hasError()) {
            stateInfo.setError(tr("IO adapter error"));
        }
    }
}

}// namespace
