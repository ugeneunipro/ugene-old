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

#include "BaseDocumentFormats.h"
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

const DocumentFormatId BaseDocumentFormats::ABIF("abi");
const DocumentFormatId BaseDocumentFormats::ACE("ace");
const DocumentFormatId BaseDocumentFormats::BAM("bam");
const DocumentFormatId BaseDocumentFormats::BED("bed");
const DocumentFormatId BaseDocumentFormats::CLUSTAL_ALN("clustal");
const DocumentFormatId BaseDocumentFormats::DATABASE_CONNECTION("database_connection");
const DocumentFormatId BaseDocumentFormats::DIFF("diff");
const DocumentFormatId BaseDocumentFormats::FASTA("fasta");
const DocumentFormatId BaseDocumentFormats::FASTQ("fastq");
const DocumentFormatId BaseDocumentFormats::FPKM_TRACKING_FORMAT("fpkm-tracking");
const DocumentFormatId BaseDocumentFormats::GFF("gff");
const DocumentFormatId BaseDocumentFormats::GTF("gtf");
const DocumentFormatId BaseDocumentFormats::INDEX("index");
const DocumentFormatId BaseDocumentFormats::MEGA("mega");
const DocumentFormatId BaseDocumentFormats::MSF("msf");
const DocumentFormatId BaseDocumentFormats::NEWICK("newick");
const DocumentFormatId BaseDocumentFormats::NEXUS("nexus");
const DocumentFormatId BaseDocumentFormats::PDW("pdw");
const DocumentFormatId BaseDocumentFormats::PHYLIP_INTERLEAVED("phylip-interleaved");
const DocumentFormatId BaseDocumentFormats::PHYLIP_SEQUENTIAL("phylip-sequential");
const DocumentFormatId BaseDocumentFormats::PLAIN_ASN("mmdb");
const DocumentFormatId BaseDocumentFormats::PLAIN_EMBL("embl");
const DocumentFormatId BaseDocumentFormats::PLAIN_GENBANK("genbank");
const DocumentFormatId BaseDocumentFormats::PLAIN_PDB("pdb");
const DocumentFormatId BaseDocumentFormats::PLAIN_SWISS_PROT("swiss-prot");
const DocumentFormatId BaseDocumentFormats::PLAIN_TEXT("text");
const DocumentFormatId BaseDocumentFormats::RAW_DNA_SEQUENCE("raw");
const DocumentFormatId BaseDocumentFormats::RTFMSA("rtfmsa");
const DocumentFormatId BaseDocumentFormats::SAM("sam");
const DocumentFormatId BaseDocumentFormats::SCF("scf");
const DocumentFormatId BaseDocumentFormats::SNP("snp");
const DocumentFormatId BaseDocumentFormats::SRF("srfasta");
const DocumentFormatId BaseDocumentFormats::STOCKHOLM("stockholm");
const DocumentFormatId BaseDocumentFormats::UGENEDB("usqlite");
const DocumentFormatId BaseDocumentFormats::VCF4("vcf");
const DocumentFormatId BaseDocumentFormats::VECTOR_NTI_SEQUENCE("Vector NTI sequence");

DocumentFormat* BaseDocumentFormats::get(const DocumentFormatId& formatId) {
    return AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
}


}//namespace
