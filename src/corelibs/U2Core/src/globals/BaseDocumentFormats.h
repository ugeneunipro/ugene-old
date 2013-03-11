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

#ifndef _U2_DOCUMENT_FORMATS_H_
#define _U2_DOCUMENT_FORMATS_H_

#include <U2Core/global.h>

namespace U2 {

#define SEQUENCE_TAG " sequence"    //common tag for sequence object
#define FEATURES_TAG " features"    //common tag for annotation table

class DocumentFormat;

class U2CORE_EXPORT BaseDocumentFormats {
public:
    static const DocumentFormatId PLAIN_TEXT;
    static const DocumentFormatId FASTA;
    static const DocumentFormatId PLAIN_GENBANK;
    static const DocumentFormatId PLAIN_EMBL;
    static const DocumentFormatId PLAIN_SWISS_PROT;
    static const DocumentFormatId ABIF;
    static const DocumentFormatId SCF;
    static const DocumentFormatId RAW_DNA_SEQUENCE;
    static const DocumentFormatId CLUSTAL_ALN;
    static const DocumentFormatId STOCKHOLM;
    static const DocumentFormatId NEWICK;
    static const DocumentFormatId PLAIN_PDB;
    static const DocumentFormatId INDEX;
    static const DocumentFormatId FASTQ;
    static const DocumentFormatId PLAIN_ASN;
    static const DocumentFormatId MSF;
    static const DocumentFormatId ACE;
    static const DocumentFormatId BED;
    static const DocumentFormatId GFF;
    static const DocumentFormatId GTF;
    static const DocumentFormatId FPKM_TRACKING_FORMAT;
    static const DocumentFormatId SRF;
    static const DocumentFormatId SAM;
    static const DocumentFormatId NEXUS;
    static const DocumentFormatId MEGA;
    static const DocumentFormatId PDW;
    static const DocumentFormatId SNP;
    static const DocumentFormatId VCF4;
    static const DocumentFormatId BAM;
    static const DocumentFormatId UGENEDB;
    static const DocumentFormatId DIFF;

    static DocumentFormat* get(const DocumentFormatId& formatId);
};


}//namespace

#endif
