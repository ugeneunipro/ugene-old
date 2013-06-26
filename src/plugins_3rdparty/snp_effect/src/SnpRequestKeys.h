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

#ifndef _U2_SNP_REQUEST_KEYS_
#define _U2_SNP_REQUEST_KEYS_

#include <QtCore/QString>
#include <QtCore/QPair>

namespace U2 {

typedef QString SnpRequestKey;

namespace SnpRequestKeys {

const SnpRequestKey R_SNP_FIRST_SEQUENCE( "-s" );
const SnpRequestKey R_SNP_SECOND_SEQUENCE( "-f" );

const SnpRequestKey R_SNP_FIRST_UNKNOWN_BINDS( "--snpdm1" );
const SnpRequestKey R_SNP_SECOND_UNKNOWN_BINDS( "--snpdm2" );

const SnpRequestKey R_SNP_SIGNIFICANCE( "--sign" );


} // namespace SnpRequestKeys

typedef QPair<QString, QString> SnpRequestDefaultItem;

typedef QString SnpRequestAddress;

namespace SnpRequestAddresses {

    const SnpRequestAddress R_SNP_TOOLS_SCRIPT_PATH( ":snp_effect/scripts/rsnp.py" );

} // namespace SnpRequestAddresses

} // namespace U2

#endif // _U2_SNP_REQUEST_KEYS_