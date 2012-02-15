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

#include "SQLiteAnnotationDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2AnnotationUtils.h>

namespace U2 {

#define ADBI_GROUP_FIELDS QString(" id, parent, sequence, name, path, rversion, lversion ")

SQLiteAnnotationDbi::SQLiteAnnotationDbi(SQLiteDbi* dbi) : U2AnnotationDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteAnnotationDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }
    SQLiteQuery("CREATE TABLE AnnotationGroup (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, parent INTEGER, sequence INTEGER NOT NULL, "
        " name TEXT NOT NULL, path TEXT NOT NULL, lversion INTEGER NOT NULL DEFAULT 0, gversion INTEGER NOT NULL DEFAULT 0, "
        " FOREIGN KEY(sequence) REFERENCES Object(id))", db, os).execute();

    SQLiteQuery("CREATE TABLE AnnotationGroupContent (group INTEGER NOT NULL, annotation INTEGER NOT NULL, "
        " FOREIGN KEY(group) REFERENCES AnnotationGroup(id), "
        " FOREIGN KEY(annotation) REFERENCES Annotation(id))", db, os).execute();

    SQLiteQuery("CREATE TABLE Annotation (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, sequence INTEGER NOT NULL, "
        " key TEXT NOT NULL, version INTEGER NOT NULL DEFAULT 0, "
        " lop INTEGER NOT NULL, strand INTEGER NOT NULL, rtype INTEGER NOT NULL, tleft INTEGER NOT NULL, tright INTEGER NOT NULL, "
        " FOREIGN KEY(sequence) REFERENCES Object(id))", db, os).execute();

    SQLiteQuery("CREATE TABLE AnnotationQualifier (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, annotation INTEGER NOT NULL, "
        " name TEXT NOT NULL, value TEXT NOT NULL, "
        " FOREIGN KEY(annotation) REFERENCES Annotation(id))", db, os).execute();

    //pos : positional order of this record in location: 0..1..2...
    SQLiteQuery("CREATE TABLE AnnotationLocation (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, annotation INTEGER NOT NULL, "
        " start INTEGER NOT NULL, len INTEGER NOT NULL, pos INTEGER NOT NULL, "
        " FOREIGN KEY(annotation) REFERENCES Annotation(id))", db, os).execute();

    //id: location id
    SQLiteQuery("CREATE VIRTUAL TABLE AnnotationLocationRTreeIndex USING rtree_i32(id, start, end)", db, os).execute();

    // TODO: create indexes
}

static U2AnnotationGroup selectGroup(SQLiteQuery& q) {
    U2AnnotationGroup res;
    if (q.step()) {
        res.id = q.getDataId(0, U2Type::AnnotationGroup);
        res.parentGroupId = q.getDataId(1, U2Type::AnnotationGroup);
        res.sequenceId = q.getDataId(2, U2Type::Sequence);
        res.name = q.getString(3);
        res.path = q.getString(4);
        res.localVersion = q.getInt64(5);
        res.globalVersion = q.getInt64(6);
    }
    return res;
}

U2AnnotationGroup SQLiteAnnotationDbi::getGroupByPath(const U2DataId& sequenceId, const QString& path, U2OpStatus& os) {
    DBI_TYPE_CHECK(sequenceId, U2Type::Sequence, os, U2AnnotationGroup());

    SQLiteQuery q("SELECT " + ADBI_GROUP_FIELDS + " FROM AnnotationGroup WHERE sequence = ?1 AND path = ?2", db, os);
    q.bindDataId(1, sequenceId);
    q.bindString(2, path);
    return selectGroup(q);
}
    
QList<U2AnnotationGroup> SQLiteAnnotationDbi::getSubgroups(const U2DataId& groupId, U2OpStatus& os) {
    QList<U2AnnotationGroup> res;
    DBI_TYPE_CHECK(groupId, U2Type::AnnotationGroup, os, res);

    SQLiteQuery q("SELECT " + ADBI_GROUP_FIELDS + " FROM AnnotationGroup WHERE parent = ?1", db, os);
    q.bindDataId(1, groupId);
    while(true) {
        U2AnnotationGroup g = selectGroup(q);
        if (g.id.isEmpty()) {
            break;
        }
        if (os.hasError()) {
            break;
        }
        res << g;
    } 
    
    return res;
}

U2AnnotationGroup SQLiteAnnotationDbi::getGroup(const U2DataId& groupId, U2OpStatus& os) {
    DBI_TYPE_CHECK(groupId, U2Type::AnnotationGroup, os, U2AnnotationGroup());

    SQLiteQuery q("SELECT " + ADBI_GROUP_FIELDS + " FROM AnnotationGroup WHERE id = ?1", db, os);
    q.bindDataId(1, groupId);
    return selectGroup(q);
}
    
U2AnnotationGroup SQLiteAnnotationDbi::createGroup(const U2DataId& sequenceId, const QString& path, U2OpStatus& os) {
    DBI_TYPE_CHECK(sequenceId, U2Type::Sequence, os, U2AnnotationGroup());

    U2AnnotationGroup g = getGroupByPath(sequenceId, path, os);
    SAFE_POINT_OP(os, U2AnnotationGroup());

    if (g.hasValidId()) {
        return g;
    }

    QStringList tokens = U2AnnotationUtils::splitPath(path, os);
    CHECK_OP(os, U2AnnotationGroup());
    
    SQLiteTransaction t(db, os);
    {
        QStringList parentTokens = tokens;
        parentTokens.removeLast();
        U2AnnotationGroup parentGroup;
        if (!tokens.isEmpty()) {
            parentGroup = createGroup(sequenceId, parentTokens.join(GROUP_PATH_SEPARATOR), os);
            SAFE_POINT_OP(os, U2AnnotationGroup());
        }

        U2AnnotationGroup result;
        result.parentGroupId = parentGroup.id;
        result.sequenceId = sequenceId;
        result.name = tokens.last();
        result.path = tokens.join(GROUP_PATH_SEPARATOR);

        SQLiteQuery q("INSERT INTO AnnotationGroup(parent, sequence, name, path) VALUES(?1, ?2, ?3, ?4)", db, os);
        q.bindDataId(1, result.parentGroupId);
        q.bindDataId(2, result.sequenceId);
        q.bindString(3, result.path);
        q.bindString(4, result.name);
        result.id = q.insert(U2Type::AnnotationGroup);
        SAFE_POINT_OP(os, U2AnnotationGroup());
        return result;
    }

}

void SQLiteAnnotationDbi::removeGroup(const U2DataId& , U2OpStatus& ) {
    //TODO: 
    assert(0);
}

static QString bindRegion(const QString& idxTable, const U2Region& r) {
    return QString(" ( %1.start < %2 AND %1.end > %3 ) ").arg(idxTable).arg(r.endPos()).arg(r.startPos);
}

qint64 SQLiteAnnotationDbi::countSequenceAnnotations(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os) {
    DBI_TYPE_CHECK(sequenceId, U2Type::Sequence, os, -1);

    if (region == U2_REGION_MAX) {
        SQLiteQuery q("SELECT COUNT(*) FROM Annotation WHERE sequence = ?1", db, os);    
        q.bindDataId(1, sequenceId);
        return q.selectInt64(-1);
    }

    // TODO: works only with 1-region annotations! To make it work with multi-region location we need to think to create second
    // rtree table for locations with id 2+ and use "select count(distinct)" slow query
    SQLiteQuery q("SELECT COUNT(*) FROM AnnotationLocationIndex AS i WHERE sequence = ?1 AND " + bindRegion("i", region) , db, os);    

    q.bindDataId(1, sequenceId);
    return q.selectInt64(-1);
}

qint64 SQLiteAnnotationDbi::countGroupAnnotations(const U2DataId& groupId, const U2Region& region, U2OpStatus& os) {
    DBI_TYPE_CHECK(groupId, U2Type::AnnotationGroup, os, -1);

    if (region == U2_REGION_MAX) {
        SQLiteQuery q("SELECT COUNT(*) FROM Annotation WHERE group = ?1", db, os);    
        q.bindDataId(1, groupId);
        return q.selectInt64(-1);
    }

    // TODO: works only with 1-region annotations! To make it work with multi-region location we need to think to create second
    // rtree table for locations with id 2+ and use "select count(distinct)" slow query
    SQLiteQuery q("SELECT COUNT(*) FROM AnnotationLocationIndex AS i WHERE group = ?1 AND " + bindRegion("i", region) , db, os);    

    q.bindDataId(1, groupId);
    return q.selectInt64(-1);
}

U2DbiIterator<U2DataId>* SQLiteAnnotationDbi::getAnnotationsBySequence(const U2DataId& sequenceId, const U2Region& region, 
                qint64 offset, qint64 count, U2OpStatus& os)
{
    //TODO: support multi-location annotations
    DBI_TYPE_CHECK(sequenceId, U2Type::Sequence, os, NULL);

    SQLiteQuery* q = new SQLiteQuery("SELECT a.id FROM Annotation AS a, AnnotationLocation AS l, AnnotationLocationIndex AS li "
                    " WHERE a.sequence = ?1 AND a.id = l.annotation AND l.id = li.id AND " + bindRegion("li", region), 
                    offset, count, db, os);
    
    q->bindDataId(1, sequenceId);
    return new SqlRSIterator<U2DataId>(q, new SqlDataIdRSLoader(U2Type::Annotation), NULL, U2DataId(), os);
}
    

U2DbiIterator<U2DataId>* SQLiteAnnotationDbi::getAnnotationsByGroup(const U2DataId& , const U2Region& ,
                qint64 , qint64 , U2OpStatus& )
{
    
    //TODO:
    assert(0);
    return NULL;
}


U2Annotation SQLiteAnnotationDbi::getAnnotation(const U2DataId& annotationId, U2OpStatus& os) {
    DBI_TYPE_CHECK(annotationId, U2Type::Annotation, os, U2Annotation());

    SQLiteQuery qa("SELECT sequenceId, key, version, lop, strand, rtype, tleft, tright FROM Annotation WHERE id = ?1" , db, os);    
    qa.bindDataId(1, annotationId);
    if (!qa.step()) {
        return U2Annotation();
    }
    CHECK_OP(os, U2Annotation());

    U2Annotation res;
    res.id = annotationId;
    res.sequenceId = qa.getDataId(0, U2Type::Sequence);
    res.key = qa.getString(1);
    res.version = qa.getInt64(2);
    res.location->op = (U2LocationOperator)qa.getInt32(3);
    res.location->strand = (qa.getInt32(4) == -1 ) ? U2Strand::Complementary : U2Strand::Direct;
    res.location->regionType = (U2LocationRegionType)qa.getInt32(5);
    res.location->truncateLeft = qa.getBool(6);
    res.location->truncateRight = qa.getBool(7);


    SQLiteQuery ql("SELECT start, len FROM AnnotationLocation WHERE annotation = ?1 ORDER BY pos" , db, os);    
    qa.bindDataId(1, annotationId);
    while (qa.step()) {
        CHECK_OP(os, U2Annotation());
        U2Region r(ql.getInt64(0), ql.getInt64(1));
        res.location->regions << r;
        
    }
    CHECK_EXT(!res.location->regions.isEmpty(), os.setError(SQLiteL10N::tr("No location found for annotation: %1").arg(SQLiteUtils::text(annotationId))), U2Annotation());
    

    SQLiteQuery qq("SELECT key, value FROM AnnotationQualifier WHERE annotation = ?1 ORDER BY id" , db, os);    
    qa.bindDataId(1, annotationId);
    while (qq.step()) {
        CHECK_OP(os, U2Annotation());
        U2Qualifier q(qq.getString(0), qq.getString(1));
        res.qualifiers << q;
    }
    return res;
}

QList<U2DataId> SQLiteAnnotationDbi::getAnnotationGroups(const U2DataId& , U2OpStatus& ) {
    QList<U2DataId> res;
    //TODO:
    assert(0);
    return res;
}

void SQLiteAnnotationDbi::createAnnotations(QList<U2Annotation>& annotations, const U2DataId& groupId, U2OpStatus& os) {
    DBI_TYPE_CHECK(groupId, U2Type::AnnotationGroup, os,);

    SQLiteTransaction t(db, os);
    SQLiteQuery qa("INSERT INTO Annotation(key, version, lop, strand, rtype, tleft, tright) VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7)" ,db, os);
    SQLiteQuery ql("INSERT INTO AnnotationLocation(annotation, start, len, pos) VALUES(?1, ?2, ?3, ?4)"  ,db, os);
    SQLiteQuery qi("INSERT INTO AnnotationLocationIndex(id, start, end) VALUES(?1, ?2, ?3)"  ,db, os);
    SQLiteQuery qq("INSERT INTO AnnotationQualifier(annotation, name, value) VALUES(?1, ?2, ?3)"  ,db, os);
    for (int i = 0, n = annotations.size(); i < n; i++) {
        U2Annotation& a = annotations[i];
        qa.reset();
        qa.bindString(1, a.key);
        qa.bindInt32(2, 0);
        qa.bindInt32(3, a.location->op);
        qa.bindInt32(4, a.location->strand.isDirect() ? 1 : -1);
        qa.bindInt32(5, a.location->regionType);
        qa.bindBool(6, a.location->truncateLeft);
        qa.bindBool(7, a.location->truncateRight);
        a.id = qa.insert(U2Type::Annotation);
        
        //location
        SAFE_POINT(a.location->regions.size() == 1, "multi-location annotations are not implemented!", );
        U2Region l1 = a.location->regions.first();
        ql.reset();
        ql.bindDataId(1, a.id);
        ql.bindInt64(2, l1.startPos);
        ql.bindInt64(3, l1.length);
        int l1Id = ql.insert();
        CHECK_OP(os, );

        //location rtree index
        qi.reset();
        qi.bindInt64(1, l1Id);
        qi.bindInt64(2, l1.startPos);
        qi.bindInt64(3, l1.endPos());
        qi.insert();
        CHECK_OP(os, );

        // qualifiers
        foreach(const U2Qualifier& q, a.qualifiers) {
            qq.reset();
            qq.bindDataId(1, a.id);
            qq.bindString(2, q.name);
            qq.bindString(3, q.value);
            qq.execute();
            CHECK_OP(os, );
        }

        addToGroup(groupId, a.id, os);
        CHECK_OP(os,);
    }
}

void SQLiteAnnotationDbi::removeAnnotation(const U2DataId& , U2OpStatus& ) {
    //TODO:
    assert(0);
}

void SQLiteAnnotationDbi::removeAnnotations(const QList<U2DataId>& , U2OpStatus& ) {
    //TODO:
    assert(0);
}
    
void SQLiteAnnotationDbi::updateLocation(const U2DataId& , const U2Location& , U2OpStatus& ) {
    //TODO:
    assert(0);
}
    
void SQLiteAnnotationDbi::updateName(const U2DataId& , const QString& , U2OpStatus& ) {
    //TODO:
    assert(0);
}
    
void SQLiteAnnotationDbi::createQualifier(const U2DataId& , const U2Qualifier& , U2OpStatus& ) {
    //TODO:
    assert(0);
}
    
void SQLiteAnnotationDbi::removeQualifier(const U2DataId& , const U2Qualifier& , U2OpStatus& ) {
    //TODO:
    assert(0);
}


bool SQLiteAnnotationDbi::isInGroup(const U2DataId& groupId, const U2DataId& annotationId, U2OpStatus& os) {
    SQLiteQuery q("SELECT group FROM AnnotationGroupContent WHERE groupId = ?1 AND annotationId = 2?", db, os);    
    q.bindDataId(1, groupId);
    q.bindDataId(2, annotationId);
    qint64 res = q.selectInt64(-1);
    CHECK_OP(os, false);
    return res >= 0;
}

void SQLiteAnnotationDbi::addToGroup(const U2DataId& groupId, const U2DataId& annotationId, U2OpStatus& os) {
    DBI_TYPE_CHECK(groupId, U2Type::AnnotationGroup, os,);
    DBI_TYPE_CHECK(annotationId, U2Type::Annotation, os,);
    CHECK(!isInGroup(groupId, annotationId, os), );
    
    SQLiteQuery q("INSERT INTO AnnotationGroupContent(group, annotation) VALUES(?1, ?2)", db, os);
    q.bindDataId(1, groupId);
    q.bindDataId(2, annotationId);
    q.execute();
}
    
void SQLiteAnnotationDbi::removeFromGroup(const U2DataId& , const U2DataId& , U2OpStatus& ) {
    //TODO:
    assert(0);
}

} //namespace
