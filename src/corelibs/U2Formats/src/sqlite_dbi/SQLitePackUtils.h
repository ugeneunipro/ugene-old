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

#ifndef _U2_SQLITE_PACKUTILS_H_
#define _U2_SQLITE_PACKUTILS_H_

#include <U2Core/U2Msa.h>

namespace U2 {
namespace SQLite {

class PackUtils {
public:
    /** Pack utils version */
    static const QByteArray VERSION;

    /** Gaps */
    static QByteArray packGaps(const QList<U2MsaGap> &gaps);
    static bool unpackGaps(const QByteArray &str, QList<U2MsaGap> &gaps);

    /** Gaps details */
    static QByteArray packGapDetails(qint64 rowId, const QList<U2MsaGap> &oldGaps, const QList<U2MsaGap> &newGaps);
    static bool unpackGapDetails(const QByteArray &modDetails, qint64 &rowId, QList<U2MsaGap> &oldGaps, QList<U2MsaGap> &newGaps);

    /** Row content details */
    static QByteArray packRowContentDetails(qint64 rowId, const QByteArray &oldSeq, const QList<U2MsaGap> &oldGaps,
        const QByteArray &newSeq, const QList<U2MsaGap> &newGaps);
    static bool unpackRowContentDetails(const QByteArray &modDetails, qint64 &rowId,
        QByteArray &oldSeq, QList<U2MsaGap> &oldGaps,
        QByteArray &newSeq, QList<U2MsaGap> &newGaps);

    /** Row order */
    static QByteArray packRowOrder(const QList<qint64>& rowIds);
    static bool unpackRowOrder(const QByteArray& str, QList<qint64>& rowsIds);

    /** Row order details */
    static QByteArray packRowOrderDetails(const QList<qint64>& oldOrder, const QList<qint64>& newOrder);
    static bool unpackRowOrderDetails(const QByteArray &modDetails, QList<qint64>& oldOrder, QList<qint64>& newOrder);

    /** Row name details */
    static QByteArray packRowNameDetails(qint64 rowId, const QString &oldName, const QString &newName);
    static bool unpackRowNameDetails(const QByteArray &modDetails, qint64 &rowId, QString &oldName, QString &newName);

    /** Row */
    static QByteArray packRow(qint64 posInMsa, const U2MsaRow& row);
    static bool unpackRow(const QByteArray &modDetails, qint64& posInMsa, U2MsaRow& row);

    /** Rows */
    static QByteArray packRows(const QList<qint64> &posInMsa, const QList<U2MsaRow> &rows);
    static bool unpackRows(const QByteArray &modDetails, QList<qint64> &posInMsa, QList<U2MsaRow> &rows);

    /** Alphabet details*/
    static QByteArray packAlphabetDetails(const U2AlphabetId &oldAlphabet, const U2AlphabetId &newAlphabet);
    static bool unpackAlphabetDetails(const QByteArray &modDetails, U2AlphabetId &oldAlphabet, U2AlphabetId &newAlphabet);

    /** Object name details */
    static QByteArray packObjectNameDetails(const QString &oldName, const QString &newName);
    static bool unpackObjectNameDetails(const QByteArray &modDetails, QString &oldName, QString &newName);

private:
    static const char SEP;
    static const char SECOND_SEP;
};

} // SQLite
} // U2

#endif // _U2_SQLITE_PACKUTILS_H_
