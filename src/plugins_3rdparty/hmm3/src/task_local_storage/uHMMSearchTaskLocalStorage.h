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

#ifndef _GB2_HMMER3_SEARCH_TLS_H_
#define _GB2_HMMER3_SEARCH_TLS_H_

#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QThreadStorage>

#include "uHMMSearchTaskLocalData.h"

namespace U2 {

struct ContextId {
    ContextId( qint64 what ) : id( what ) {}
    qint64 id;
}; // ContextId

class UHMM3SearchTaskLocalStorage {
public:
    static const UHMM3SearchTaskLocalData* current();
    
    static UHMM3SearchTaskLocalData* createTaskContext( qint64 ctxId );
    
    static void freeTaskContext( qint64 ctxId );
    
private:
    static QHash< qint64, UHMM3SearchTaskLocalData* > data;
    static QThreadStorage< ContextId* > tls;
    static QMutex mutex;
    static const UHMM3SearchTaskLocalData defaultData;
    
}; // UHMMSearchTaskLocalStorage

} // U2

#endif // _GB2_HMMER3_SEARCH_TLS_H_
