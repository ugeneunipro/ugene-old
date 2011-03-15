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

#ifndef _U2_DBI_UTILS_H_
#define _U2_DBI_UTILS_H_

#include <U2Core/U2Dbi.h>
#include <U2Core/Task.h>

namespace U2 {

/** Default (empty) implementation for optional DBI methods */
class U2CORE_EXPORT U2AbstractDbi : public U2Dbi {
public:
    U2AbstractDbi();
    
    virtual bool flush(U2OpStatus& os) {return true;}

    virtual U2DbiState getState() const {return state;}

protected:
    U2DbiState state;
};


/** 
    Helper class that allocates connection in constructor and automatically releases it in the destructor 
    It uses app-global connection pool.

    Note: DbiHandle caches U2OpStatus and reuses it in destructor on DBI release. Ensure that 
    U2OpStatus live range contains DbiHandle live range
*/
class U2CORE_EXPORT DbiHandle {
public:
    /** Opens connection to existing DBI */
    DbiHandle(U2DbiFactoryId id, const QString& url,  U2OpStatus& os);
    
    /** Opens connection to existing DBI or create news DBI*/
    DbiHandle(U2DbiFactoryId id, const QString& url,  bool create, U2OpStatus& os);

    ~DbiHandle();

    U2Dbi* dbi;
    U2OpStatus& os;
};



}// namespace

#endif
