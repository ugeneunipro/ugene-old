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

const SnpRequestKey SNP_FEATURE_ID_KEY( "featureID" );

const SnpRequestKey R_SNP_FIRST_SEQUENCE( "-f" );
const SnpRequestKey R_SNP_SECOND_SEQUENCE( "-s" );
const SnpRequestKey R_SNP_FIRST_SITE_STATE( "--snpdm1" );
const SnpRequestKey R_SNP_SECOND_SITE_STATE( "--snpdm2" );
const SnpRequestKey R_SNP_SIGNIFICANCE( "--sign" );

const SnpRequestKey PROT_STAB_1D_SEQUENCE( "-s" );
const SnpRequestKey PROT_STAB_1D_MUTATION_POS( "-p" );
const SnpRequestKey PROT_STAB_1D_REPLACEMENT( "-t" );

const SnpRequestKey PROT_STAB_3D_PDB_ID( "-i" );
const SnpRequestKey PROT_STAB_3D_CHAIN( "-c" );
const SnpRequestKey PROT_STAB_3D_MUTATION_POS( "-p" );
const SnpRequestKey PROT_STAB_3D_REPLACEMENT( "-t" );

const SnpRequestKey SNP_2_PDB_SITE_PDB_ID( "" );
const SnpRequestKey SNP_2_PDB_SITE_CHAIN( "" );
const SnpRequestKey SNP_2_PDB_SITE_MUTATIONS( "" );

const SnpRequestKey SNP_CHIP_TOOLS_UG( "" );
const SnpRequestKey SNP_CHIP_TOOLS_SNP_ID( "" );

} // namespace SnpRequestKeys

typedef QString SnpResponseKey;

namespace SnpResponseKeys {
const SnpResponseKey DEFAULT_SEPARATOR(":");

const SnpResponseKey R_SNP_PRESENT_TFBS( "PRESENT" );
const SnpResponseKey PROT_STABILITY_1D( "STABILITY_1D" );
const SnpResponseKey PROT_STABILITY_3D( "STABILITY_3D" );
const SnpResponseKey SNP_2_PDB_SITE_( "PDB_SITE" );
// TODO: add response keys for SNPChIP tool
const SnpResponseKey SNP_CHIP_TOOLS_( "" );

}

namespace SnpRequestingScripts {

const QString R_SNP_TOOLS_SCRIPT( "rsnp.py" );
const QString SNP_PROT_STABILITY_1D_SCRIPT( "protstability1d.py" );
const QString SNP_PROT_STABILITY_3D_SCRIPT( "protstability3d.py" );
const QString SNP_2_PDB_SITE_SCRIPT( "snp2pdbsite" );
const QString SNP_CHIP_TOOLS_SCRIPT( "" );

} // namespace SnpRequestAddresses

} // namespace U2

#endif // _U2_SNP_REQUEST_KEYS_