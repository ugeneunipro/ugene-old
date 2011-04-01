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

#include "SQLiteSequenceDbi.h"
#include "SQLiteObjectDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/Timer.h>

namespace U2 {

SQLiteSequenceDbi::SQLiteSequenceDbi(SQLiteDbi* dbi) : U2SequenceDbi(dbi), SQLiteChildDBICommon(dbi) {
}

U2Sequence SQLiteSequenceDbi::getSequenceObject(const U2DataId& sequenceId, U2OpStatus& os) {
    U2Sequence res(sequenceId, dbi->getDbiId(), 0);
    SQLiteQuery q("SELECT Sequence.length, Sequence.alphabet, Sequence.circular, Object.version FROM Sequence, Object "
        " WHERE Object.id = ?1 AND Sequence.object = Object.id", db, os);
    q.bindDataId(1, sequenceId);
    if (q.step()) {
        res.length = q.getInt64(0);
        res.alphabet = q.getString(1);
        res.circular = q.getBool(2);
        res.version = q.getInt64(3);
        q.ensureDone();
    }
    return res;
}

QByteArray SQLiteSequenceDbi::getSequenceData(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os) {
    GTIMER(c1, t1, "SQLiteSequenceDbi::getSequenceData");
    GCOUNTER(c2, t2, "SQLiteSequenceDbi::getSequenceData -> calls");
    QByteArray res;
    res.reserve(region.length);
    SQLiteQuery q("SELECT sstart, send, data FROM SequenceData WHERE sequence = ?1 "
            "AND  ((sstart >= ?2 AND sstart <= ?3) OR (?2 >= sstart AND send > ?2)) ORDER BY sstart", db, os);
    q.bindDataId(1, sequenceId);
    q.bindInt64(2, region.startPos);
    q.bindInt64(3, region.endPos());
    qint64 pos = region.startPos;
    while (q.step()) {
        qint64 sstart = q.getInt64(0);
        qint64 send = q.getInt64(1);
        qint64 length = send - sstart;
        QByteArray data = q.getBlob(2);

        int copyStart = pos - sstart;
        int copyLength = qMin(region.length, length - copyStart);
        res.append(data.constData() + copyStart, copyLength);
        pos+=copyLength;
    }
    return res;
}



void SQLiteSequenceDbi::createSequenceObject(U2Sequence& sequence, const QString& folder, U2OpStatus& os) {
    sequence.id = SQLiteObjectDbi::createObject(U2Type::Sequence, folder, sequence.visualName, SQLiteDbiObjectRank_TopLevel, db, os);
    if (os.hasError()) {
        return;
    }
    SQLiteQuery q("INSERT INTO Sequence(object, alphabet) VALUES(?1, ?2)", db, os);
    q.bindDataId(1, sequence.id);
    q.bindString(2, sequence.alphabet.id);
    q.execute();
}

#define SEQUENCE_CHUNK_SIZE (1024*1024)

static QList<QByteArray> quantify(const QList<QByteArray> input) {
    QList<QByteArray> res;
    QByteArray currentChunk;
    foreach (const QByteArray& i, input) {
        int d = SEQUENCE_CHUNK_SIZE - currentChunk.length();
        if (i.length() <= d) { //if 'i' fits into chunk - just add it
            currentChunk.append(i);
        } else  { // 'i' does not fit into chunk -> split it into separate chunks
            for (int j = 0; j < i.length(); j += d) {
                if (j > 0) {
                    d = qMin(SEQUENCE_CHUNK_SIZE, i.length() - j);
                }
                currentChunk.append(i.constData() + j, + d);
                assert(currentChunk.length() == SEQUENCE_CHUNK_SIZE);
                res.append(currentChunk);
                currentChunk.clear();
            }
        }
        if (currentChunk.length() == SEQUENCE_CHUNK_SIZE) {
            res.append(currentChunk);
            currentChunk.clear();
        }
    }
    if (!currentChunk.isEmpty())  {
        res.append(currentChunk);
    }
    return res;

}
void SQLiteSequenceDbi::updateSequenceData(const U2DataId& sequenceId, const U2Region& regionToReplace, const QByteArray& dataToInsert, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    //algorithm: 
        // find all regions affected -> remove them
        // construct new regions from cuts from old regions and new dataToInsert
        // remove affected annotations or adjust their locations if possible
    
    // find cropped parts
    QByteArray leftCrop;
    SQLiteQuery leftQ("SELECT sstart FROM SequenceData WHERE sequence = ?1 AND sstart <= ?2 AND send >= ?2", db, os);
    leftQ.bindDataId(1, sequenceId);
    leftQ.bindInt64(2, regionToReplace.startPos);
    qint64 cropLeftPos = leftQ.selectInt64(-1);
    if (os.hasError()) {
        return;
    }
    if (cropLeftPos >= 0) {
        leftCrop = getSequenceData(sequenceId, U2Region(cropLeftPos, regionToReplace.startPos - cropLeftPos), os);
    }

    QByteArray rightCrop;
    SQLiteQuery rightQ("SELECT send FROM SequenceData WHERE sequence = ?1 AND sstart <= ?2 AND send >= ?2", db, os);
    rightQ.bindDataId(1, sequenceId);
    rightQ.bindInt64(2, regionToReplace.endPos());
    qint64 cropRightPos = rightQ.selectInt64(-1);
    if (os.hasError()) {
        return;
    }
    if (cropRightPos > 0) {
        rightCrop = getSequenceData(sequenceId, U2Region(regionToReplace.endPos(), cropRightPos - regionToReplace.endPos()), os);
    }
    
    // remove all affected regions
    SQLiteQuery removeQ("DELETE FROM SequenceData WHERE sequence = ?1 "
        " AND ((sstart >= ?2 AND sstart <= ?3) OR (?2 >= sstart  AND send > ?2))", db, os);
    removeQ.bindDataId(1, sequenceId);
    removeQ.bindInt64(2, regionToReplace.startPos);
    removeQ.bindInt64(3, regionToReplace.endPos());
    removeQ.execute();
    if (os.hasError()) {
        return;
    }

    // now adjust 'sstart' & 'send' for all sequence regions on the right side of the insert
    if (dataToInsert.length() != regionToReplace.length) {
        qint64 d = dataToInsert.length() - regionToReplace.length;
        SQLiteQuery updateQ("UPDATE SequenceData SET sstart = sstart + ?1, send = send + ?1 WHERE sequence = ?2 AND sstart >= ?3", db, os);
        updateQ.bindInt64(1, d);
        updateQ.bindDataId(2, sequenceId);
        updateQ.bindInt64(3, regionToReplace.endPos());
        updateQ.execute();
        if (os.hasError()) {
            return;
        }

    }

    // insert new regions
    QList<QByteArray> newDataToInsert = quantify(QList<QByteArray>() << leftCrop << dataToInsert << rightCrop);
    SQLiteQuery insertQ("INSERT INTO SequenceData(sequence, sstart, send, data) VALUES(?1, ?2, ?3, ?4)", db, os);
    qint64 startPos = regionToReplace.startPos;
    foreach(const QByteArray& d, newDataToInsert) {
        insertQ.reset();
        insertQ.bindDataId(1, sequenceId);
        insertQ.bindInt64(2, startPos);
        insertQ.bindInt64(3, startPos + d.length());
        insertQ.bindBlob(4, d);
        insertQ.execute();
        if (os.hasError()) {
            return;
        }
        startPos += d.length();
    }
    // Update sequence object length;
    SQLiteQuery lengthQ("SELECT MAX(send) FROM SequenceData WHERE sequence = ?1", db, os);
    lengthQ.bindDataId(1, sequenceId);
    qint64 newLength = lengthQ.selectInt64();
    if (os.hasError()) {
        return;
    }
    SQLiteQuery updateLengthQuery("UPDATE Sequence SET length = ?1 WHERE object = ?2", db, os);
    updateLengthQuery.bindInt64(1, newLength);
    updateLengthQuery.bindDataId(2, sequenceId);
    updateLengthQuery.update(1);
    if (os.hasError()) {
        return;
    }

    SQLiteObjectDbi::incrementVersion(sequenceId, db, os);
                      }

} //namespace
