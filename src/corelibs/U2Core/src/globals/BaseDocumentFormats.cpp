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

#include "BaseDocumentFormats.h"
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

const DocumentFormatId BaseDocumentFormats::PLAIN_TEXT("text");
const DocumentFormatId BaseDocumentFormats::FASTA("fasta");
const DocumentFormatId BaseDocumentFormats::PLAIN_GENBANK("genbank");
const DocumentFormatId BaseDocumentFormats::PLAIN_EMBL("embl");
const DocumentFormatId BaseDocumentFormats::PLAIN_SWISS_PROT("swiss-prot");
const DocumentFormatId BaseDocumentFormats::ABIF("abi");
const DocumentFormatId BaseDocumentFormats::SCF("scf");
const DocumentFormatId BaseDocumentFormats::PLAIN_PDB("pdb");
const DocumentFormatId BaseDocumentFormats::RAW_DNA_SEQUENCE("raw");
const DocumentFormatId BaseDocumentFormats::CLUSTAL_ALN("clustal");
const DocumentFormatId BaseDocumentFormats::STOCKHOLM("stockholm");
const DocumentFormatId BaseDocumentFormats::NEWICK("newick");
const DocumentFormatId BaseDocumentFormats::INDEX("index");
const DocumentFormatId BaseDocumentFormats::FASTQ("fastq");
const DocumentFormatId BaseDocumentFormats::PLAIN_ASN("mmdb");
const DocumentFormatId BaseDocumentFormats::MSF("msf");
const DocumentFormatId BaseDocumentFormats::GFF("gff");
const DocumentFormatId BaseDocumentFormats::SRF("srfasta");
const DocumentFormatId BaseDocumentFormats::SAM("sam");
const DocumentFormatId BaseDocumentFormats::NEXUS("nexus");
const DocumentFormatId BaseDocumentFormats::MEGA("mega");
const DocumentFormatId BaseDocumentFormats::ACE("ace");
const DocumentFormatId BaseDocumentFormats::PDW("pdw");

DocumentFormat* BaseDocumentFormats::get(const DocumentFormatId& formatId) {
    return AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
}


}//namespace
