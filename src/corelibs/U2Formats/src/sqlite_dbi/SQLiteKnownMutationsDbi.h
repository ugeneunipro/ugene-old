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



#ifndef _U2_SQLITE_KNOWN_MUTATIONS_DBI_H_
#define _U2_SQLITE_KNOWN_MUTATIONS_DBI_H_


#include <U2Formats/SQLiteDbi.h>

#include <U2Core/KnownMutationsDbi.h>

namespace U2 {
    

class SQLiteKnownMutationsDbi  : public KnownMutationsDbi, public SQLiteChildDBICommon{
public:
    SQLiteKnownMutationsDbi(SQLiteDbi* rootDbi);

    void initSqlSchema(U2OpStatus& os);

    /* Creates known mutation track instance */
    virtual void createKnownMutationsTrack(KnownMutationsTrack& mutationsTrack, U2OpStatus& os);

    /* Remove the known mutation track and its mutations from the database */
    virtual void removeKnownMutationsTrack(const KnownMutationsTrack& mutationsTrack, U2OpStatus& os);

    /* returns known mutation tracks by given chromosome number (x-23, y-24)*/
    virtual KnownMutationsTrack getKnownMutationsTrack(int chrNumber, U2OpStatus& os);

    /* Insert known mutations of the specific track into db*/
    virtual void addKnownMutationsToTrack (const U2DataId& mTrack, U2DbiIterator<KnownMutation>* it, U2OpStatus& os);

    /* Get all KnownMutations in track*/
    virtual U2DbiIterator<KnownMutation>* getKnownMutations(const U2DataId& mTrack, U2OpStatus& os);

    /* Get KnownMutations by startPos*/
    virtual U2DbiIterator<KnownMutation>* getKnownMutations(const U2DataId& mTrack, qint64 startPos, U2OpStatus& os);

    /* Get KnownMutations count in the track*/
    virtual int getKnownMutationsCount(const U2DataId& mTrack, U2OpStatus& os);
};
    
} //namespace

#endif //_U2_SQLITE_KNOWN_MUTATIONS_DBI_H_
