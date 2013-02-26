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

#include <U2Core/U2SafePoints.h>

#include "SQLitePackUtils.h"

namespace U2 {
namespace SQLite {

const QByteArray PackUtils::VERSION("0");
const char PackUtils::SEP = '\t';

QByteArray PackUtils::packGaps(const QList<U2MsaGap> &gaps) {
    QByteArray result;
    foreach (const U2MsaGap &gap, gaps) {
        if (!result.isEmpty()) {
            result += ";";
        }
        result += QByteArray::number(gap.offset);
        result += ",";
        result += QByteArray::number(gap.gap);
    }
    return "\"" + result + "\"";
}

bool PackUtils::unpackGaps(const QByteArray &str, QList<U2MsaGap> &gaps) {
    CHECK(str.startsWith('\"') && str.endsWith('\"'), false);
    QByteArray gapsStr = str.mid(1, str.length() - 2);
    if (gapsStr.isEmpty()) {
        return true;
    }

    QList<QByteArray> tokens = gapsStr.split(';');
    foreach (const QByteArray &t, tokens) {
        QList<QByteArray> gapTokens = t.split(',');
        CHECK(2 == gapTokens.size(), false);
        bool ok = false;
        U2MsaGap gap;
        gap.offset = gapTokens[0].toLongLong(&ok);
        CHECK(ok, false);
        gap.gap = gapTokens[1].toLongLong(&ok);
        CHECK(ok, false);
        gaps << gap;
    }
    return true;
}

QByteArray PackUtils::packGapDetails(qint64 rowId, const QList<U2MsaGap> &oldGaps, const QList<U2MsaGap> &newGaps) {
    QByteArray result = VERSION;
    result += SEP;
    result += QByteArray::number(rowId);
    result += SEP;
    result += packGaps(oldGaps);
    result += SEP;
    result += packGaps(newGaps);
    return result;
}

bool PackUtils::unpackGapDetails(const QByteArray &modDetails, qint64 &rowId, QList<U2MsaGap> &oldGaps, QList<U2MsaGap> &newGaps) {
    QList<QByteArray> tokens = modDetails.split(SEP);
    SAFE_POINT(4 == tokens.size(), QString("Invalid gap modDetails string '%1'").arg(QString(modDetails)), false);
    { // version
        SAFE_POINT(VERSION == tokens[0], QString("Invalid modDetails version '%1'").arg(tokens[0].data()), false);
    }
    { // rowId
        bool ok = false;
        rowId = tokens[1].toLongLong(&ok);
        SAFE_POINT(ok, QString("Invalid gap modDetails rowId '%1'").arg(tokens[1].data()), false);
    }
    { // oldGaps
        bool ok = unpackGaps(tokens[2], oldGaps);
        SAFE_POINT(ok, QString("Invalid gap string '%1'").arg(tokens[2].data()), false);
    }
    { // newGaps
        bool ok = unpackGaps(tokens[3], newGaps);
        SAFE_POINT(ok, QString("Invalid gap string '%1'").arg(tokens[3].data()), false);
    }
    return true;
}

QByteArray PackUtils::packRowContentDetails(qint64 rowId, const QByteArray &oldSeq, const QList<U2MsaGap> &oldGaps,
    const QByteArray &newSeq, const QList<U2MsaGap> &newGaps) {
    QByteArray result = VERSION;
    result += SEP;
    result += QByteArray::number(rowId);
    result += SEP;
    result += oldSeq;
    result += SEP;
    result += packGaps(oldGaps);
    result += SEP;
    result += newSeq;
    result += SEP;
    result += packGaps(newGaps);
    return result;
}

bool PackUtils::unpackRowContentDetails(const QByteArray &modDetails, qint64 &rowId,
    QByteArray &oldSeq, QList<U2MsaGap> &oldGaps,
    QByteArray &newSeq, QList<U2MsaGap> &newGaps) {
    QList<QByteArray> tokens = modDetails.split(SEP);
    SAFE_POINT(6 == tokens.size(), QString("Invalid row content modDetails string '%1'").arg(QString(modDetails)), false);
    { // version
        SAFE_POINT(VERSION == tokens[0], QString("Invalid modDetails version '%1'").arg(tokens[0].data()), false);
    }
    { // rowId
        bool ok = false;
        rowId = tokens[1].toLongLong(&ok);
        SAFE_POINT(ok, QString("Invalid row content modDetails rowId '%1'").arg(tokens[1].data()), false);
    }
    { // oldSeq
        oldSeq = tokens[2];
    }
    { // oldGaps
        bool ok = unpackGaps(tokens[3], oldGaps);
        SAFE_POINT(ok, QString("Invalid row content string '%1'").arg(tokens[3].data()), false);
    }
    { // newSeq
        newSeq = tokens[4];
    }
    { // newGaps
        bool ok = unpackGaps(tokens[5], newGaps);
        SAFE_POINT(ok, QString("Invalid row content string '%1'").arg(tokens[5].data()), false);
    }
    return true;
}

QByteArray PackUtils::packRowOrder(const QList<qint64>& rowIds) {
    QByteArray result;
    foreach (qint64 rowId, rowIds) {
        if (!result.isEmpty()) {
            result += ",";
        }
        result += QByteArray::number(rowId);
    }
    return "\"" + result + "\"";
}

bool PackUtils::unpackRowOrder(const QByteArray& str, QList<qint64>& rowsIds) {
    CHECK(str.startsWith('\"') && str.endsWith('\"'), false);
    QByteArray orderStr = str.mid(1, str.length() - 2);
    if (orderStr.isEmpty()) {
        return true;
    }

    QList<QByteArray> tokens = orderStr.split(',');
    foreach (const QByteArray &t, tokens) {
        bool ok = false;
        rowsIds << t.toLongLong(&ok);
        CHECK(ok, false);
    }
    return true;
}

QByteArray PackUtils::packRowOrderDetails(const QList<qint64>& oldOrder, const QList<qint64>& newOrder) {
    QByteArray result = VERSION;
    result += SEP;
    result += packRowOrder(oldOrder);
    result += SEP;
    result += packRowOrder(newOrder);
    return result;
}

bool PackUtils::unpackRowOrderDetails(const QByteArray &modDetails, QList<qint64>& oldOrder, QList<qint64>& newOrder) {
    QList<QByteArray> tokens = modDetails.split(SEP);
    SAFE_POINT(3 == tokens.size(), QString("Invalid rows order modDetails string '%1'").arg(QString(modDetails)), false);
    { // vesrion
        SAFE_POINT(VERSION == tokens[0], QString("Invalid modDetails version '%1'").arg(tokens[0].data()), false);
    }
    { // oldOrder
        bool ok = unpackRowOrder(tokens[1], oldOrder);
        SAFE_POINT(ok, QString("Invalid rows order string '%1'").arg(tokens[1].data()), false);
    }
    { // newGaps
        bool ok = unpackRowOrder(tokens[2], newOrder);
        SAFE_POINT(ok, QString("Invalid rows order string '%1'").arg(tokens[2].data()), false);
    }

    return true;
}

QByteArray PackUtils::packRowNameDetails(qint64 rowId, const QString &oldName, const QString &newName) {
    QByteArray result = VERSION;
    result += SEP;
    result += QByteArray::number(rowId);
    result += SEP;
    result += oldName.toLatin1();
    result += SEP;
    result += newName.toLatin1();
    return result;
}

bool PackUtils::unpackRowNameDetails(const QByteArray &modDetails, qint64 &rowId, QString &oldName, QString &newName) {
    QList<QByteArray> tokens = modDetails.split(SEP);
    SAFE_POINT(4 == tokens.size(), QString("Invalid row name modDetails string '%1'").arg(QString(modDetails)), false);
    { // version
        SAFE_POINT(VERSION == tokens[0], QString("Invalid modDetails version '%1'").arg(tokens[0].data()), false);
    }
    { // rowId
        bool ok = false;
        rowId = tokens[1].toLongLong(&ok);
        SAFE_POINT(ok, QString("Invalid row name modDetails rowId '%1'").arg(tokens[1].data()), false);
    }
    { // oldName
        oldName = tokens[2];
    }
    { // newName
        newName = tokens[3];
    }
    return true;
}

QByteArray PackUtils::packAddedRow(qint64 posInMsa, const U2MsaRow& row) {
    QByteArray result = VERSION;
    result += SEP;
    result += QByteArray::number(posInMsa);
    result += SEP;
    result += QByteArray::number(row.rowId);
    result += SEP;
    result += row.sequenceId.toHex();
    result += SEP;
    result += QByteArray::number(row.gstart);
    result += SEP;
    result += QByteArray::number(row.gend);
    result += SEP;
    result += packGaps(row.gaps);
    return result;
}

bool PackUtils::unpackAddedRow(const QByteArray &modDetails, qint64& posInMsa, U2MsaRow& row) {
    QList<QByteArray> tokens = modDetails.split(SEP);
    SAFE_POINT(7 == tokens.size(), QString("Invalid added row modDetails string '%1'").arg(QString(modDetails)), false);
    { // version
        SAFE_POINT(VERSION == tokens[0], QString("Invalid modDetails version '%1'").arg(tokens[0].data()), false);
    }
    { // posInMsa
        bool ok = false;
        posInMsa = tokens[1].toLongLong(&ok);
        SAFE_POINT(ok, QString("Invalid added row modDetails posInMsa '%1'").arg(tokens[1].data()), false);
    }
    { // rowId
        bool ok = false;
        row.rowId = tokens[2].toLongLong(&ok);
        SAFE_POINT(ok, QString("Invalid added row modDetails rowId '%1'").arg(tokens[2].data()), false);
    }
    { // sequenceId
        row.sequenceId = QByteArray::fromHex(tokens[3]);
    }
    { // gstart
        bool ok = false;
        row.gstart = tokens[4].toLongLong(&ok);
        SAFE_POINT(ok, QString("Invalid added row modDetails gstart '%1'").arg(tokens[4].data()), false);
    }
    { // gend
        bool ok = false;
        row.gend = tokens[5].toLongLong(&ok);
        SAFE_POINT(ok, QString("Invalid added row modDetails gend '%1'").arg(tokens[5].data()), false);
    }
    { // gaps
        bool ok = unpackGaps(tokens[6], row.gaps);
        SAFE_POINT(ok, QString("Invalid added row modDetails gaps '%1'").arg(tokens[6].data()), false);
    }
    return true;
}

QByteArray PackUtils::packAlphabetDetails(const U2AlphabetId &oldAlphabet, const U2AlphabetId &newAlphabet) {
    QByteArray result = VERSION;
    result += SEP;
    result += oldAlphabet.id.toLatin1();
    result += SEP;
    result += newAlphabet.id.toLatin1();
    return result;
}

bool PackUtils::unpackAlphabetDetails(const QByteArray &modDetails, U2AlphabetId &oldAlphabet, U2AlphabetId &newAlphabet) {
    QList<QByteArray> tokens = modDetails.split(SEP);
    SAFE_POINT(3 == tokens.count(), QString("Invalid modDetails '%1'!").arg(QString(modDetails)), false);
    SAFE_POINT(VERSION == tokens[0], QString("Invalid modDetails version '%1'").arg(QString(tokens[0])), false);

    oldAlphabet = QString(tokens[1]);
    newAlphabet = QString(tokens[2]);

    if (!oldAlphabet.isValid() || !newAlphabet.isValid()) {
        return false;
    }

    return true;
}

QByteArray PackUtils::packObjectNameDetails(const QString &oldName, const QString &newName) {
    QByteArray result = VERSION;
    result += SEP;
    result += oldName.toLatin1();
    result += SEP;
    result += newName.toLatin1();
    return result;
}

bool PackUtils::unpackObjectNameDetails(const QByteArray &modDetails, QString &oldName, QString &newName) {
    QList<QByteArray> tokens = modDetails.split(SEP);
    SAFE_POINT(3 == tokens.count(), "Invalid modDetails!", false);
    SAFE_POINT(VERSION == tokens[0], "Invalid modDetails version!", false);
    SAFE_POINT(!QString(tokens[1]).isEmpty(), "Invalid modDetails!", false);
    SAFE_POINT(!QString(tokens[2]).isEmpty(), "Invalid modDetails!", false);

    oldName = tokens[1];
    newName = tokens[2];
    return true;
}

} // SQLite
} // U2
