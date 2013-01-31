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

#ifndef _U2_MALIGNMENT_EXPORTER_H_
#define _U2_MALIGNMENT_EXPORTER_H_

#include <U2Core/MAlignment.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatus.h>


namespace U2 {

/** Getting a multiple alignment from DBI */
class U2CORE_EXPORT MAlignmentExporter {
public:
    MAlignmentExporter();

    MAlignment getAlignment(const U2DbiRef& dbiRef, const U2DataId& msaId, U2OpStatus& os);

private:
    QList<U2MsaRow> exportRows(const U2DataId&, U2OpStatus&);
    QList<DNASequence> exportSequencesOfRows(QList<U2MsaRow>, U2OpStatus&);
    QVariantMap exportAlignmentInfo(const U2DataId&, U2OpStatus&);
    U2Msa exportAlignmentObject(const U2DataId&, U2OpStatus&);

    MAlignment            al;
    DbiConnection         con;
};

} // namespace

#endif
