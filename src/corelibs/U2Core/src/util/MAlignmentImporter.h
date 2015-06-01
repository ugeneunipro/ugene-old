/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MALIGNMENT_IMPORTER_H_
#define _U2_MALIGNMENT_IMPORTER_H_

#include <U2Core/U2Msa.h>
#include <U2Core/U2Sequence.h>

namespace U2 {

class DbiConnection;
class MAlignment;
class MAlignmentObject;
class U2DbiRef;
class U2OpStatus;

/** Importing a multiple alignment into db */
class U2CORE_EXPORT MAlignmentImporter {
public:
    /**
     * Creates an alignment in the db.
     * The alignment is completely removed in case of an error.
     */
    static MAlignmentObject * createAlignment(const U2DbiRef &dbiRef, MAlignment &al, U2OpStatus &os);
    static MAlignmentObject * createAlignment(const U2DbiRef &dbiRef, const QString &folder, MAlignment &al,
        U2OpStatus &os, const QList<U2Sequence> &alignedSequences = QList<U2Sequence>());

private:
    static U2Msa importMsaObject(const DbiConnection &con, const QString &folder, const MAlignment &al, U2OpStatus &os);
    static void importMsaInfo(const DbiConnection &con, const U2DataId &msaId, const MAlignment &al, U2OpStatus &os);
    static QList<U2Sequence> importSequences(const DbiConnection &con, const QString &folder, const MAlignment &al, U2OpStatus &os);
    static void setChildRankForSequences(const DbiConnection &con, const QList<U2Sequence> &sequences, U2OpStatus &os);
    static QList<U2MsaRow> importRows(const DbiConnection &con, MAlignment &al, U2Msa &msa, const QList<U2Sequence> &rows, U2OpStatus &os);
};

} // namespace

#endif
