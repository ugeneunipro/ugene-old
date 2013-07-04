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

#include "SchemeSimilarityUtils.h"

#include "CInterfaceSasTests.h"

static const QString WORKING_DIR = U2::AppContext::getWorkingDirectoryPath( );
static const QString PROPER_WD_SCHEMES_PATH = WORKING_DIR + "/../../test/_common_data/cmdline/wd-sas-schemes/";

namespace U2 {

/////////////   TESTS IMPLEMENTATION   ///////////////////////////////////////////////////////    

IMPLEMENT_TEST( CInterfaceSasTests, align_with_clustalO_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "ClustalO", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Alignment", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "align_with_clustalO.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, align_with_clustalW_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "clustalw", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Alignment", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "align_with_clustalW.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, align_with_kalign_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "kalign", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Alignment", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "align_with_kalign.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, align_with_mafft_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "mafft", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Alignment", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "align_with_mafft.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, align_with_muscle_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "muscle", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Alignment", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "align_with_muscle.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, align_with_tcoffee_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "tcoffee", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Alignment", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "align_with_tcoffee.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, annotate_with_uql_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "query", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "annotate_with_uql.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceSasTests, basic_align_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "muscle", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Write Alignment", "document-format", "stockholm" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "basic_align.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceSasTests, build_weight_matrix_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "wmatrix-build", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Alignment", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "build_weight_matrix.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, cd_search_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "cd-search", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "cd_search.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}    
IMPLEMENT_TEST( CInterfaceSasTests, dna_statistics_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "dna-stats", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "dna_statistics.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceSasTests, faqual2fastq_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "import-phred-qualities", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Write Sequence", "document-format", "fastq" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "faqual2fastq.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceSasTests, filter_annotations_by_name_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "filter-annotations", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Annotations", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "filter_annotations_by_name.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, find_repeats_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "repeats-search", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Find Repeats", "algorithm", "0" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Find Repeats", "exclude-tandems", "false" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Find Repeats", "filter-algorithm", "0" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Find Repeats", "identity", "100" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Find Repeats", "inverted", "false" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Find Repeats", "max-distance", "5000" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Find Repeats", "min-distance", "0" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Find Repeats", "min-length", "5" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Find Repeats", "threads", "0" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "find_repeats.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, hmm2_build_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "hmm2-build", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Alignment", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "hmm2_build.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, import_phred_qualities_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "import-phred-qualities", NULL, NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "import_phred_qualities.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceSasTests, join_sequences_into_alignment_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "sequences-to-msa", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "join_sequences_into_alignment.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceSasTests, local_blast_search_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "blast", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "local_blast_search.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, local_blast_plus_search_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "blast-plus", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "local_blast_plus_search.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, merge_annotations_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "import-annotations", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Annotations", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "merge_annotations.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, merge_assemblies_with_cuffmerge_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "cuffmerge", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Annotations", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "merge_assemblies_with_cuffmerge.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, orf_marker_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "orf-search", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "orf_marker.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, remote_blast_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "blast-ncbi", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "remote_blast.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, reverse_complement_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "reverse-complement", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Sequence", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "reverse_complement.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, split_alignment_into_sequences_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "convert-alignment-to-sequence", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Alignment", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "split_alignment_into_sequences.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, split_assembly_into_sequences_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "assembly-to-sequences", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Assembly", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "split_assembly_into_sequences.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}
IMPLEMENT_TEST( CInterfaceSasTests, text2sequence_sas ) {
    SchemeHandle scheme = NULL;
    U2ErrorType error = createSas( "convert-text-to-sequence", NULL, NULL, &scheme    );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, "Read Plain Text", "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "text2sequence.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

} // namespace U2