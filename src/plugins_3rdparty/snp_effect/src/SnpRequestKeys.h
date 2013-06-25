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

const SnpRequestKey R_SNP_FIRST_SEQUENCE( "snpf1" );
const SnpRequestKey R_SNP_SECOND_SEQUENCE( "snpf2" );

} // namespace SnpRequestKeys

typedef QPair<QString, QString> SnpRequestDefaultItem;

namespace SnpRequestDefaultItems {

const SnpRequestDefaultItem R_SNP_RUN_SIGNAL = SnpRequestDefaultItem( "b1", "Calculate" );

} // namespace SnpRequestDefaultItems

typedef QString SnpRequestAddress;

namespace SnpRequestAddresses {

const SnpRequestAddress R_SNP_TOOLS_ADDRESS( "http://samurai.bionet.nsc.ru/cgi-bin/03/programs/rsnp_lin/rsnpd.pl" );

} // namespace SnpRequestAddresses

} // namespace U2

#endif // _U2_SNP_REQUEST_KEYS_