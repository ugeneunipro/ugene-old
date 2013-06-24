
#include "SQLiteKnownMutationsDbi.h"
#include "SQLiteObjectDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>

namespace U2{

SQLiteKnownMutationsDbi::SQLiteKnownMutationsDbi( SQLiteDbi* rootDbi )
: KnownMutationsDbi(rootDbi)
, SQLiteChildDBICommon(rootDbi)
{

}

void SQLiteKnownMutationsDbi::initSqlSchema( U2OpStatus& os ){
    if (os.hasError()) {
        return;
    }

    // KnownMutationsTrack object
    SQLiteQuery(" CREATE TABLE KnownMutationsTrack (object INTEGER, chromosome INTEGER NOT NULL, "
        " FOREIGN KEY(object) REFERENCES Object(id))", db, os).execute();
}

void SQLiteKnownMutationsDbi::createKnownMutationsTrack( KnownMutationsTrack& mutationsTrack, U2OpStatus& os ){
    if (mutationsTrack.chrNumber == 0) {
        os.setError(SQLiteL10N::tr("chrNumber is not set!"));
        return;
    }

    dbi->getSQLiteObjectDbi()->createObject(mutationsTrack, "", SQLiteDbiObjectRank_TopLevel, os);
    SAFE_POINT_OP(os,);

    SQLiteQuery q1("INSERT INTO KnownMutationsTrack(object, chromosome) VALUES(?1, ?2)", db, os);
    q1.bindDataId(1, mutationsTrack.id);
    q1.bindInt32(2, mutationsTrack.chrNumber);
    q1.execute();

    //in one KnownMutation_%1 table are stored only mutations of %1 track for performance reasons
    // KnownMutation entity
    // track - KnownMutationsTrack object id
    // startPos - mutation start position
    // refData - reference sequence part
    // obsData - observed variation of the reference
    // dbSnpId - identifier visible for user
    // scores in different dbs
    //gene position
    QString knownTableString = QString("table_%1").arg(SQLiteUtils::toDbiId(mutationsTrack.id));

    QString createMutString = QString("CREATE TABLE KnownMutation_%1 (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, track INTEGER, startPos INTEGER, "
        " refData BLOB NOT NULL, obsData BLOB NOT NULL, dbSnpId BLOB NOT NULL, "
        " avSift  DOUBLE, lrt DOUBLE, phylop DOUBLE, pp2 DOUBLE, mt DOUBLE, genomes1000 DOUBLE, segmentalDuplication DOUBLE, conserved DOUBLE, "
        " gerpConserved DOUBLE, allFreq DOUBLE, hapmap DOUBLE, gerpScore DOUBLE,"
        " genePosition DOUBLE, "
        " FOREIGN KEY(track) REFERENCES KnownMutationsTrack(object) )").arg(knownTableString);
    SQLiteQuery(createMutString, db, os).execute();

    QString indexString = QString("CREATE INDEX IF NOT EXISTS KnownMutationIndex_%1 ON KnownMutation_%1(startPos)" ).arg(knownTableString);
    SQLiteQuery(indexString, db, os).execute();

    SAFE_POINT_OP(os,);
}

void SQLiteKnownMutationsDbi::removeKnownMutationsTrack( const KnownMutationsTrack& mutationsTrack, U2OpStatus& os ){
    QString knownTableString = QString("table_%1").arg(SQLiteUtils::toDbiId(mutationsTrack.id));
    SQLiteQuery q1(QString("DELETE FROM KnownMutation_%1").arg(knownTableString), db, os);
    q1.execute();
    SAFE_POINT_OP(os,);

    SQLiteQuery q2("DELETE FROM KnownMutationsTrack WHERE object = ?1", db, os);
    q2.bindDataId(1, mutationsTrack.id);
    q2.execute();
    SAFE_POINT_OP(os,);
}

KnownMutationsTrack SQLiteKnownMutationsDbi::getKnownMutationsTrack( int chrNumber, U2OpStatus& os ){
    KnownMutationsTrack res;

    SQLiteQuery q("SELECT object, chromosome FROM KnownMutationsTrack WHERE chromosome = ?1", db, os);
    q.bindInt32(1, chrNumber);
    if (q.step())  {
        res.id = q.getDataId(0, U2Type::KnownMutationsTrackType);
        res.chrNumber = q.getInt32(1);
    } 
    return res;
}

void SQLiteKnownMutationsDbi::addKnownMutationsToTrack( const U2DataId& mTrack, U2DbiIterator<KnownMutation>* it, U2OpStatus& os ){

    QString knownTableString = QString("table_%1").arg(SQLiteUtils::toDbiId(mTrack));

    SQLiteQuery q2(QString("INSERT INTO KnownMutation_%1(track, startPos, refData, obsData, dbSnpId, "
        " avSift, lrt, phylop, pp2, mt, genomes1000, segmentalDuplication, conserved, gerpConserved, allFreq, hapmap, gerpScore, genePosition) " 
        " VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18)").arg(knownTableString), db, os);
    while (it->hasNext() && !os.isCoR()) {
        KnownMutation mut = it->next();
        q2.reset();
        q2.bindDataId(1, mTrack);
        q2.bindInt64(2, mut.startPos);
        q2.bindBlob(3, mut.refData);
        q2.bindBlob(4, mut.obsData);
        q2.bindBlob(5, mut.dbSnpId);
        q2.bindDouble(6, mut.avSift);
        q2.bindDouble(7, mut.ljb_lrt);
        q2.bindDouble(8, mut.ljb_phylop);
        q2.bindDouble(9, mut.ljb_pp2);
        q2.bindDouble(10, mut.ljb_mt);
        q2.bindDouble(11, mut.genomes1000);
        q2.bindDouble(12, mut.segmentalDuplication);
        q2.bindDouble(13, mut.conserved);
        q2.bindDouble(14, mut.gerpConserved);
        q2.bindDouble(15, mut.allFreq);
        q2.bindDouble(16, mut.hapmap);
        q2.bindDouble(17, mut.gerpScore);
        q2.bindDouble(18, mut.genePosition);
        q2.execute();
        SAFE_POINT_OP(os,);
    }
}
class SimpleKnownMutationsLoader: public SqlRSLoader<KnownMutation> {
public:
    KnownMutation load(SQLiteQuery* q) {
        KnownMutation res;

        res.startPos = q->getInt64(0);
        res.refData = q->getBlob(1);
        res.obsData = q->getBlob(2);
        res.dbSnpId = q->getBlob(3);

        res.avSift = q->getDouble(4);
        res.ljb_lrt = q->getDouble(5);
        res.ljb_phylop = q->getDouble(6);
        res.ljb_pp2 = q->getDouble(7);
        res.ljb_mt = q->getDouble(8);
        res.genomes1000 = q->getDouble(9);
        res.segmentalDuplication = q->getDouble(10);
        res.conserved = q->getDouble(11);
        res.gerpConserved = q->getDouble(12);
        res.allFreq = q->getDouble(13);
        res.hapmap = q->getDouble(14);
        res.gerpScore = q->getDouble(15);
        res.genePosition = q->getDouble(16);

        return res;
    }
};

U2DbiIterator<KnownMutation>* SQLiteKnownMutationsDbi::getKnownMutations( const U2DataId& mTrack, U2OpStatus& os ){
    QString knownTableString = QString("table_%1").arg(SQLiteUtils::toDbiId(mTrack));
    QSharedPointer<SQLiteQuery> q  (new SQLiteQuery(QString("SELECT startPos, refData, obsData, dbSnpId, "
        "avSift, lrt, phylop, pp2, mt, genomes1000, segmentalDuplication, conserved, gerpConserved, allFreq, hapmap, gerpScore, genePosition \
        FROM KnownMutation_%1 \
        ").arg(knownTableString) , db, os));
    return new SqlRSIterator<KnownMutation>(q, new SimpleKnownMutationsLoader(), NULL, KnownMutation(), os);
}

U2DbiIterator<KnownMutation>* SQLiteKnownMutationsDbi::getKnownMutations( const U2DataId& mTrack, qint64 startPos, U2OpStatus& os ){
    QString knownTableString = QString("table_%1").arg(SQLiteUtils::toDbiId(mTrack));
    SQLiteTransaction t(db, os);
    QSharedPointer<SQLiteQuery> q = t.getPreparedQuery(QString("SELECT startPos, refData, obsData, dbSnpId, "
        "avSift, lrt, phylop, pp2, mt, genomes1000, segmentalDuplication, conserved, gerpConserved, allFreq, hapmap, gerpScore, genePosition \
        FROM KnownMutation_%1 \
        WHERE startPos = ?1").arg(knownTableString) , db, os);
    q->bindInt64(1, startPos);
    return new SqlRSIterator<KnownMutation>(q, new SimpleKnownMutationsLoader(), NULL, KnownMutation(), os);
}

int SQLiteKnownMutationsDbi::getKnownMutationsCount( const U2DataId& mTrack, U2OpStatus& os ){
    QString knownTableString = QString("table_%1").arg(SQLiteUtils::toDbiId(mTrack));
    SQLiteQuery q(QString("SELECT COUNT(*) FROM KnownMutation_%1").arg(knownTableString) , db, os);
    if (!q.step()) {
        return -1;
    }

    return q.getInt32(0); 
}

}//namespace
