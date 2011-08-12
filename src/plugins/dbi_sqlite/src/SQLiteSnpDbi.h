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

#ifndef _U2_SQLITE_SNP_DBI_H_
#define _U2_SQLITE_SNP_DBI_H_

#include "SQLiteDbi.h"

#include <U2Core/U2SnpDbi.h>

namespace U2 {

class SQLiteSnpDbi : public U2SnpDbi, public SQLiteChildDBICommon {
public:

    SQLiteSnpDbi(SQLiteDbi* dbi);

    virtual U2DbiIterator<U2SnpTrack>* getSnpTracks(U2OpStatus& os);
    
    /** Returns SnpTrack instance by the given id */
    virtual U2SnpTrack getSnpTrack(const U2DataId& id, U2OpStatus& os);

    /** 
        Creates new SnpTrack instance and adds all snps provided by iterator to it 
        Requires U2DbiFeature_WriteSnps feature support
    */
    virtual void createSnpTrack(U2SnpTrack& track, U2DbiIterator<U2Snp> *it, const QString& folder, U2OpStatus& os);
 
    /** 
        Updates SnpTrack instance 
        Requires U2DbiFeature_WriteSnps feature support
    */
    virtual void updateSnpTrack(const U2SnpTrack& track, U2OpStatus& os);

    /** Returns all Snps from the given region */
    virtual U2DbiIterator<U2Snp>* getSnps(const U2DataId& track, const U2Region& region, U2OpStatus& os);

    virtual void initSqlSchema(U2OpStatus& os);

};


} //namespace

#endif
