/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <U2Core/MAlignment.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatus.h>


namespace U2 {

/** Importing a multiple alignment into db */
class U2CORE_EXPORT MAlignmentImporter {
public:
    /**
     * Creates an alignment in the db.
     * The alignment is completely removed in case of an error.
     */
    static U2EntityRef createAlignment(const U2DbiRef&, const MAlignment&, U2OpStatus&);

private:
    static U2Msa importMsaObject(const DbiConnection&, const MAlignment&, U2OpStatus&);
    static void importMsaInfo(const DbiConnection&, const U2DataId& msaId, const MAlignment&, U2OpStatus&);
    static QList<U2Sequence> importSequences(const DbiConnection&, const MAlignment&, U2OpStatus&);
    static QList<U2MsaRow> importRows(const DbiConnection&, const MAlignment&, U2Msa&, QList<U2Sequence>, U2OpStatus&);
    static void makeSequencesChildObjects(const DbiConnection&, QList<U2Sequence>, U2OpStatus&);
};

} // namespace

#endif
