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

#include "CInterfaceManualTests.h"

static const QString PROPER_WD_SCHEMES_PATH = PATH_PREFIX_DATA + QString( ":workflow_samples" );

namespace U2 {

/////////////   TESTS IMPLEMENTATION   ///////////////////////////////////////////////////////

IMPLEMENT_TEST( CInterfaceManualTests, basic_align ) {
    char readMsa[MAX_ELEMENT_NAME_LENGTH], muscle[MAX_ELEMENT_NAME_LENGTH],
        writeMsa[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "read-msa", MAX_ELEMENT_NAME_LENGTH, readMsa );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-msa", MAX_ELEMENT_NAME_LENGTH, writeMsa );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "muscle", MAX_ELEMENT_NAME_LENGTH, muscle );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeMsa, "document-format", "stockholm" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, readMsa, "out-msa", muscle, "in-msa" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, muscle, "out-msa", writeMsa, "in-msa" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readMsa, "msa", muscle, "in-msa.msa" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, muscle, "msa", writeMsa, "in-msa.msa" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/Alignment/basic_align.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, faqual2fastq ) {
    char readSequence[MAX_ELEMENT_NAME_LENGTH], importPhredQualities[MAX_ELEMENT_NAME_LENGTH],
        writeSequence[MAX_ELEMENT_NAME_LENGTH];

    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-sequence", MAX_ELEMENT_NAME_LENGTH, writeSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "import-phred-qualities", MAX_ELEMENT_NAME_LENGTH,
        importPhredQualities );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "document-format", "fastq" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, readSequence, "out-sequence", importPhredQualities,
        "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, importPhredQualities, "out-sequence", writeSequence,
        "in-sequence" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readSequence, "sequence", importPhredQualities,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, importPhredQualities, "sequence", writeSequence,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/Conversions/faqual2fastq.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, msa2clustal ) {
    char readMsa[MAX_ELEMENT_NAME_LENGTH], writeMsa[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "read-msa", MAX_ELEMENT_NAME_LENGTH, readMsa );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-msa", MAX_ELEMENT_NAME_LENGTH, writeMsa );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeMsa, "document-format", "clustal" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeMsa, "write-mode", "2" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readMsa, "msa", writeMsa, "in-msa.msa" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/Conversions/msa2clustal.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, query2alignment ) {
    char readSequence[MAX_ELEMENT_NAME_LENGTH], query[MAX_ELEMENT_NAME_LENGTH],
        extractAnnotatedSequence[MAX_ELEMENT_NAME_LENGTH], sequencesToMsa[MAX_ELEMENT_NAME_LENGTH],
        writeMsa[MAX_ELEMENT_NAME_LENGTH], filterAnnotations[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "query", MAX_ELEMENT_NAME_LENGTH, query );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "extract-annotated-sequence", MAX_ELEMENT_NAME_LENGTH,
        extractAnnotatedSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "sequences-to-msa", MAX_ELEMENT_NAME_LENGTH,
        sequencesToMsa );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-msa", MAX_ELEMENT_NAME_LENGTH, writeMsa );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "filter-annotations", MAX_ELEMENT_NAME_LENGTH,
        filterAnnotations );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readSequence, "merge-gap", "10" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, readSequence, "mode", "0" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, query, "merge", "false" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, query, "offset", "0" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, extractAnnotatedSequence, "complement", "true" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, extractAnnotatedSequence, "extend-left", "0" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, extractAnnotatedSequence, "extend-right", "0" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, extractAnnotatedSequence, "merge-gap-length", "1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, extractAnnotatedSequence, "translate", "false" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, writeMsa, "document-format", "clustal" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeMsa, "write-mode", "2" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, filterAnnotations, "accept-or-filter", "true" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readSequence, "sequence", query, "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, filterAnnotations, "annotations",
        extractAnnotatedSequence, "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", extractAnnotatedSequence,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, extractAnnotatedSequence, "sequence", sequencesToMsa,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, sequencesToMsa, "msa", writeMsa, "in-msa.msa" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, query, "annotations", filterAnnotations,
        "in-annotations.annotations" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/Conversions/query2alignment.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, seq2gen ) {
    char readSequence[MAX_ELEMENT_NAME_LENGTH], writeSequence[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-sequence", MAX_ELEMENT_NAME_LENGTH, writeSequence );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readSequence, "merge-gap", "10" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, readSequence, "mode", "0" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, writeSequence, "document-format", "genbank" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "accumulate", "true" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "write-mode", "2" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readSequence, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", writeSequence,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/Conversions/seq2gen.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, tfbs ) {
    char readSequence[MAX_ELEMENT_NAME_LENGTH], writeSequence[MAX_ELEMENT_NAME_LENGTH],
        multiplexer[MAX_ELEMENT_NAME_LENGTH], siteconSearch[MAX_ELEMENT_NAME_LENGTH],
        wmatrixSearch[MAX_ELEMENT_NAME_LENGTH], siteconRead[MAX_ELEMENT_NAME_LENGTH],
        wmatrixRead[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-sequence", MAX_ELEMENT_NAME_LENGTH, writeSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "multiplexer", MAX_ELEMENT_NAME_LENGTH, multiplexer );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "sitecon-search", MAX_ELEMENT_NAME_LENGTH, siteconSearch );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "wmatrix-search", MAX_ELEMENT_NAME_LENGTH, wmatrixSearch );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "sitecon-read", MAX_ELEMENT_NAME_LENGTH, siteconRead );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "wmatrix-read", MAX_ELEMENT_NAME_LENGTH, wmatrixRead );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, writeSequence, "document-format", "genbank" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, siteconSearch, "result-name", "sitecon" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, wmatrixSearch, "result-name", "weight_matrix" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, multiplexer, "output-data", writeSequence, "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, siteconSearch, "out-annotations", multiplexer, "input-data-2" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, wmatrixSearch, "out-annotations", multiplexer, "input-data-1" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, siteconRead, "out-sitecon", siteconSearch, "in-sitecon" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, wmatrixRead, "out-wmatrix", wmatrixSearch, "in-wmatrix" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, readSequence, "out-sequence", wmatrixSearch, "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, readSequence, "out-sequence", siteconSearch, "in-sequence" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readSequence, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, siteconSearch, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, wmatrixSearch, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", writeSequence,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", siteconSearch,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, siteconRead, "sitecon-model", siteconSearch,
        "in-sitecon.sitecon-model" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", wmatrixSearch,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, wmatrixRead, "wmatrix", wmatrixSearch,
        "in-wmatrix.wmatrix" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/Data merging/tfbs.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, build_test_HMM ) {
    char readSequence[MAX_ELEMENT_NAME_LENGTH], readMsa[MAX_ELEMENT_NAME_LENGTH],
        hmm2Build[MAX_ELEMENT_NAME_LENGTH], hmm2WriteProfile[MAX_ELEMENT_NAME_LENGTH],
        hmm2Search[MAX_ELEMENT_NAME_LENGTH], writeSequence[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "read-msa", MAX_ELEMENT_NAME_LENGTH, readMsa );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "hmm2-build", MAX_ELEMENT_NAME_LENGTH, hmm2Build );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "hmm2-write-profile", MAX_ELEMENT_NAME_LENGTH,
        hmm2WriteProfile );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "hmm2-search", MAX_ELEMENT_NAME_LENGTH, hmm2Search );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-sequence", MAX_ELEMENT_NAME_LENGTH, writeSequence );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readSequence, "merge-gap", "10" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, readSequence, "mode", "0" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, hmm2Build, "calibrate", "true" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Build, "calibration-threads", "1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Build, "deviation", "200" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Build, "fix-samples-length", "0" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Build, "mean-samples-length", "325" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Build, "samples-num", "5000" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Build, "seed", "0" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Build, "strategy", "1" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, hmm2WriteProfile, "write-mode", "2" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, hmm2Search, "e-val", "-1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Search, "result-name", "hmm_signal" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Search, "score", "-1000000000" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Search, "seqs-num", "1" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, writeSequence, "document-format", "genbank" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "accumulate", "true" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "write-mode", "2" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readMsa, "msa", hmm2Build, "in-msa.msa" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, hmm2Build, "hmm2-profile", hmm2WriteProfile,
        "in-hmm2.hmm2-profile" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, hmm2Build, "hmm2-profile", hmm2Search,
        "in-hmm2.hmm2-profile" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", hmm2Search,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, hmm2Search, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", writeSequence,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/HMMER/build-test-HMM.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, search_HMM ) {
    char readSequence[MAX_ELEMENT_NAME_LENGTH], hmm2ReadProfile[MAX_ELEMENT_NAME_LENGTH],
        hmm2Search[MAX_ELEMENT_NAME_LENGTH], writeSequence[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "hmm2-read-profile", MAX_ELEMENT_NAME_LENGTH,
        hmm2ReadProfile );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "hmm2-search", MAX_ELEMENT_NAME_LENGTH, hmm2Search );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-sequence", MAX_ELEMENT_NAME_LENGTH, writeSequence );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readSequence, "merge-gap", "10" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, readSequence, "mode", "0" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, hmm2Search, "e-val", "-1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Search, "result-name", "hmm_signal" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Search, "score", "-1000000000" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, hmm2Search, "seqs-num", "1" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, writeSequence, "document-format", "genbank" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "accumulate", "true" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "write-mode", "2" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, hmm2ReadProfile, "hmm2-profile", hmm2Search,
        "in-hmm2.hmm2-profile" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", hmm2Search,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, hmm2Search, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", writeSequence,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/HMMER/searchHMM.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, search_TFBS ) {
    char readSequence[MAX_ELEMENT_NAME_LENGTH], writeSequence[MAX_ELEMENT_NAME_LENGTH],
        siteconSearch[MAX_ELEMENT_NAME_LENGTH], siteconSearch1[MAX_ELEMENT_NAME_LENGTH],
        siteconSearch2[MAX_ELEMENT_NAME_LENGTH], siteconRead[MAX_ELEMENT_NAME_LENGTH],
        siteconRead1[MAX_ELEMENT_NAME_LENGTH], siteconRead2[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-sequence", MAX_ELEMENT_NAME_LENGTH, writeSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "sitecon-search", MAX_ELEMENT_NAME_LENGTH, siteconSearch );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "sitecon-search", MAX_ELEMENT_NAME_LENGTH, siteconSearch1 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "sitecon-search", MAX_ELEMENT_NAME_LENGTH, siteconSearch2 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "sitecon-read", MAX_ELEMENT_NAME_LENGTH, siteconRead );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "sitecon-read", MAX_ELEMENT_NAME_LENGTH, siteconRead1 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "sitecon-read", MAX_ELEMENT_NAME_LENGTH, siteconRead2 );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, writeSequence, "document-format", "genbank" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, siteconSearch, "strand", "0" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, siteconSearch1, "strand", "0" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, siteconSearch2, "strand", "0" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, siteconRead1, "out-sitecon", siteconSearch1, "in-sitecon" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, siteconSearch1, "out-annotations", siteconSearch2,
        "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, siteconRead, "out-sitecon", siteconSearch, "in-sitecon" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, siteconSearch2, "out-annotations", writeSequence,
        "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, siteconSearch, "out-annotations", siteconSearch1,
        "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, siteconRead2, "out-sitecon", siteconSearch2, "in-sitecon" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, readSequence, "out-sequence", siteconSearch, "in-sequence" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readSequence, "sequence", siteconSearch,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, siteconRead, "sitecon-model", siteconSearch,
        "in-sitecon.sitecon-model" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", siteconSearch1,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, siteconRead1, "sitecon-model", siteconSearch1,
        "in-sitecon.sitecon-model" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", siteconSearch2,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, siteconRead2, "sitecon-model", siteconSearch2,
        "in-sitecon.sitecon-model" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, siteconSearch2, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, siteconSearch, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, siteconSearch1, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", writeSequence,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/Transcriptomics/SearchTFBS.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, call_variants ) {
    char readSequence[MAX_ELEMENT_NAME_LENGTH], readAssembly[MAX_ELEMENT_NAME_LENGTH],
        callVariants[MAX_ELEMENT_NAME_LENGTH], writeVariations[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "read-assembly", MAX_ELEMENT_NAME_LENGTH, readAssembly );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "call_variants", MAX_ELEMENT_NAME_LENGTH, callVariants );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-variations", MAX_ELEMENT_NAME_LENGTH, writeVariations );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readSequence, "url-in.dataset", "Dataset" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, readAssembly, "url-in.dataset", "Dataset" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, writeVariations, "document-format", "snp" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeVariations, "write-mode", "0" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, readAssembly, "out-assembly", callVariants, "in-assembly" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, readSequence, "out-sequence", callVariants, "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, callVariants, "out-variations", writeVariations, "in-variations" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, callVariants, "variation-track", writeVariations,
        "in-variations.variation-track" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readAssembly, "assembly", callVariants,
        "in-assembly.assembly" );
    error = addSchemeActorsBinding( scheme, readAssembly, "url", callVariants, "in-assembly.url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", callVariants,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "url", callVariants, "in-sequence.url" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/NGS/call_variants.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, chip_seq ) {
    char readAnnotations[MAX_ELEMENT_NAME_LENGTH], macsId[MAX_ELEMENT_NAME_LENGTH],
        ceasReport[MAX_ELEMENT_NAME_LENGTH], conservationPlotId[MAX_ELEMENT_NAME_LENGTH],
        seqPosId[MAX_ELEMENT_NAME_LENGTH], peak2GeneId[MAX_ELEMENT_NAME_LENGTH],
        conductGoId[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-annotations", MAX_ELEMENT_NAME_LENGTH,
        readAnnotations );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "macs-id", MAX_ELEMENT_NAME_LENGTH, macsId );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "ceas-report", MAX_ELEMENT_NAME_LENGTH, ceasReport );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "conservation_plot-id", MAX_ELEMENT_NAME_LENGTH,
        conservationPlotId );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "seqpos-id", MAX_ELEMENT_NAME_LENGTH, seqPosId );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "peak2gene-id", MAX_ELEMENT_NAME_LENGTH, peak2GeneId );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "conduct-go-id", MAX_ELEMENT_NAME_LENGTH, conductGoId );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readAnnotations, "url-in.dataset", "Dataset" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, readAnnotations, "out-annotations", macsId, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, macsId, "out-data", ceasReport, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, macsId, "out-data", conservationPlotId, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, macsId, "out-data", seqPosId, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, macsId, "out-data", peak2GeneId, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, peak2GeneId, "out-data", conductGoId, "in-data" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readAnnotations, "annotations", macsId,
        "in-data._treatment-ann" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, macsId, "wiggle-treat", ceasReport,
        "in-data.enrichment-signal" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, macsId, "peak-regions", ceasReport,
        "in-data.peak-regions" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, macsId, "peak-regions", conservationPlotId,
        "in-data.cp_treat-ann" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, macsId, "peak-regions", seqPosId,
        "in-data.cp_treat-ann" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, macsId, "peak-regions", peak2GeneId,
        "in-data._treat-ann" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, peak2GeneId, "gene-annotation", conductGoId,
        "in-data.in-ann" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/NGS/cistrome/chip_seq.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, chip_seq_with_control ) {
    char readAnnotations1[MAX_ELEMENT_NAME_LENGTH], readAnnotations2[MAX_ELEMENT_NAME_LENGTH],
        macsId[MAX_ELEMENT_NAME_LENGTH], ceasReport[MAX_ELEMENT_NAME_LENGTH],
        conservationPlotId[MAX_ELEMENT_NAME_LENGTH], seqPosId[MAX_ELEMENT_NAME_LENGTH],
        peak2GeneId[MAX_ELEMENT_NAME_LENGTH], conductGoId[MAX_ELEMENT_NAME_LENGTH],
        multiplexer[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-annotations", MAX_ELEMENT_NAME_LENGTH,
        readAnnotations1 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "read-annotations", MAX_ELEMENT_NAME_LENGTH,
        readAnnotations2 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "macs-id", MAX_ELEMENT_NAME_LENGTH, macsId );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "ceas-report", MAX_ELEMENT_NAME_LENGTH, ceasReport );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "conservation_plot-id", MAX_ELEMENT_NAME_LENGTH,
        conservationPlotId );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "seqpos-id", MAX_ELEMENT_NAME_LENGTH, seqPosId );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "peak2gene-id", MAX_ELEMENT_NAME_LENGTH, peak2GeneId );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "conduct-go-id", MAX_ELEMENT_NAME_LENGTH, conductGoId );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "multiplexer", MAX_ELEMENT_NAME_LENGTH, multiplexer );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readAnnotations1, "url-in.dataset", "Dataset" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, readAnnotations2, "url-in.dataset", "Dataset" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, readAnnotations1, "out-annotations", multiplexer,
        "input-data-1" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, readAnnotations2, "out-annotations", multiplexer,
        "input-data-2" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, multiplexer, "output-data", macsId, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, macsId, "out-data", ceasReport, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, macsId, "out-data", conservationPlotId, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, macsId, "out-data", seqPosId, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, macsId, "out-data", peak2GeneId, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, peak2GeneId, "out-data", conductGoId, "in-data" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readAnnotations1, "annotations", macsId,
        "in-data._treatment-ann" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readAnnotations2, "annotations", macsId,
        "in-data.control-ann" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, macsId, "wiggle-treat", ceasReport,
        "in-data.enrichment-signal" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, macsId, "peak-regions", ceasReport,
        "in-data.peak-regions" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, macsId, "peak-regions", conservationPlotId,
        "in-data.cp_treat-ann" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, macsId, "peak-regions", seqPosId,
        "in-data.cp_treat-ann" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, macsId, "peak-regions", peak2GeneId,
        "in-data._treat-ann" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, peak2GeneId, "gene-annotation", conductGoId,
        "in-data.in-ann" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/NGS/cistrome/chip_seq_with_control.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, tuxedo_main ) {
    char getFileList[MAX_ELEMENT_NAME_LENGTH], topHat[MAX_ELEMENT_NAME_LENGTH],
        cufflinks[MAX_ELEMENT_NAME_LENGTH], cuffmerge[MAX_ELEMENT_NAME_LENGTH],
        cuffdiff[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "get-file-list", MAX_ELEMENT_NAME_LENGTH, getFileList );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "cuffdiff", MAX_ELEMENT_NAME_LENGTH, cuffdiff );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "cuffmerge", MAX_ELEMENT_NAME_LENGTH, cuffmerge );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "cufflinks", MAX_ELEMENT_NAME_LENGTH, cufflinks );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "tophat", MAX_ELEMENT_NAME_LENGTH, topHat );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, getFileList, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, getFileList, "url-in.dataset", "Dataset 2" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, getFileList, "out-url", topHat, "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, cufflinks, "out-annotations", cuffmerge, "in-annotations" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, cuffmerge, "out-annotations", cuffdiff, "in-annotations" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, topHat, "out-assembly", cufflinks, "in-assembly" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, topHat, "out-assembly", cuffdiff, "in-assembly" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, getFileList, "dataset", topHat,
        "in-sequence.dataset" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, getFileList, "url", topHat, "in-sequence.in-url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, topHat, "hits-url", cufflinks, "in-assembly.url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, cufflinks, "isolevel.slot", cuffmerge,
        "in-annotations.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, cuffmerge, "annotations", cuffdiff,
        "in-annotations.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, topHat, "hits-url", cuffdiff, "in-assembly.url" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/NGS/tuxedo/tuxedo_main.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, tuxedo_main_paired ) {
    char getFileList1[MAX_ELEMENT_NAME_LENGTH], getFileList2[MAX_ELEMENT_NAME_LENGTH],
        topHat[MAX_ELEMENT_NAME_LENGTH], cufflinks[MAX_ELEMENT_NAME_LENGTH],
        cuffmerge[MAX_ELEMENT_NAME_LENGTH], cuffdiff[MAX_ELEMENT_NAME_LENGTH],
        multiplexer[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "get-file-list", MAX_ELEMENT_NAME_LENGTH, getFileList1 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "get-file-list", MAX_ELEMENT_NAME_LENGTH, getFileList2 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "cuffdiff", MAX_ELEMENT_NAME_LENGTH, cuffdiff );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "cuffmerge", MAX_ELEMENT_NAME_LENGTH, cuffmerge );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "cufflinks", MAX_ELEMENT_NAME_LENGTH, cufflinks );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "tophat", MAX_ELEMENT_NAME_LENGTH, topHat );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "multiplexer", MAX_ELEMENT_NAME_LENGTH, multiplexer );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, getFileList1, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, getFileList1, "url-in.dataset", "Dataset 2" );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, getFileList2, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, getFileList2, "url-in.dataset", "Dataset 2" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, topHat, "out-assembly", cufflinks, "in-assembly" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, topHat, "out-assembly", cuffdiff, "in-assembly" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, getFileList2, "out-url", multiplexer, "input-data-2" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, getFileList1, "out-url", multiplexer, "input-data-1" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, multiplexer, "output-data", topHat, "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, cufflinks, "out-annotations", cuffmerge, "in-annotations" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, cuffmerge, "out-annotations", cuffdiff, "in-annotations" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, getFileList1, "dataset", topHat,
        "in-sequence.dataset" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, getFileList1, "url", topHat, "in-sequence.in-url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, getFileList2, "url", topHat,
        "in-sequence.paired-url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, topHat, "hits-url", cufflinks, "in-assembly.url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, cufflinks, "isolevel.slot", cuffmerge,
        "in-annotations.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, cuffmerge, "annotations", cuffdiff,
        "in-annotations.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, topHat, "hits-url", cuffdiff, "in-assembly.url" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/NGS/tuxedo/tuxedo_main_paired.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, tuxedo_multiple_dataset ) {
    char getFileList[MAX_ELEMENT_NAME_LENGTH], readAnnotations[MAX_ELEMENT_NAME_LENGTH],
        topHat[MAX_ELEMENT_NAME_LENGTH], cuffdiff[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "get-file-list", MAX_ELEMENT_NAME_LENGTH, getFileList );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "read-annotations", MAX_ELEMENT_NAME_LENGTH, readAnnotations );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "cuffdiff", MAX_ELEMENT_NAME_LENGTH, cuffdiff );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "tophat", MAX_ELEMENT_NAME_LENGTH, topHat );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readAnnotations, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, getFileList, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, getFileList, "url-in.dataset", "Dataset 2" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, topHat, "out-assembly", cuffdiff, "in-assembly" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, getFileList, "out-url", topHat, "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, readAnnotations, "out-annotations", cuffdiff, "in-annotations" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, getFileList, "dataset", topHat,
        "in-sequence.dataset" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, getFileList, "url", topHat, "in-sequence.in-url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readAnnotations, "annotations", cuffdiff,
        "in-annotations.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, topHat, "hits-url", cuffdiff, "in-assembly.url" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/NGS/tuxedo/tuxedo_no_novel_transcr.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, tuxedo_multiple_dataset_paired ) {
    char getFileList1[MAX_ELEMENT_NAME_LENGTH], getFileList2[MAX_ELEMENT_NAME_LENGTH],
        readAnnotations[MAX_ELEMENT_NAME_LENGTH], topHat[MAX_ELEMENT_NAME_LENGTH],
        cuffdiff[MAX_ELEMENT_NAME_LENGTH], multiplexer[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "get-file-list", MAX_ELEMENT_NAME_LENGTH, getFileList1 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "get-file-list", MAX_ELEMENT_NAME_LENGTH, getFileList2 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "read-annotations", MAX_ELEMENT_NAME_LENGTH,
        readAnnotations );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "cuffdiff", MAX_ELEMENT_NAME_LENGTH, cuffdiff );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "tophat", MAX_ELEMENT_NAME_LENGTH, topHat );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "multiplexer", MAX_ELEMENT_NAME_LENGTH, multiplexer );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readAnnotations, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, getFileList1, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, getFileList1, "url-in.dataset", "Dataset 2" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, getFileList2, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, getFileList2, "url-in.dataset", "Dataset 2" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, topHat, "out-assembly", cuffdiff, "in-assembly" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, getFileList1, "out-url", multiplexer, "input-data-1" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, getFileList2, "out-url", multiplexer, "input-data-2" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, multiplexer, "output-data", topHat, "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, readAnnotations, "out-annotations", cuffdiff,
        "in-annotations" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, getFileList1, "dataset", topHat,
        "in-sequence.dataset" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, getFileList1, "url", topHat, "in-sequence.in-url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, getFileList2, "url", topHat,
        "in-sequence.paired-url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readAnnotations, "annotations", cuffdiff,
        "in-annotations.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, topHat, "hits-url", cuffdiff, "in-assembly.url" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/NGS/tuxedo/tuxedo_no_novel_transcr_paired.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, tuxedo_single_dataset ) {
    char getFileList[MAX_ELEMENT_NAME_LENGTH], topHat[MAX_ELEMENT_NAME_LENGTH],
        cufflinks[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "get-file-list", MAX_ELEMENT_NAME_LENGTH, getFileList );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "cufflinks", MAX_ELEMENT_NAME_LENGTH, cufflinks );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "tophat", MAX_ELEMENT_NAME_LENGTH, topHat );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, getFileList, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, topHat, "out-assembly", cufflinks, "in-assembly" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, getFileList, "out-url", topHat, "in-sequence" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, getFileList, "dataset", topHat,
        "in-sequence.dataset" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, getFileList, "url", topHat, "in-sequence.in-url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, topHat, "hits-url", cufflinks, "in-assembly.url" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/NGS/tuxedo/tuxedo_single_dataset.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, tuxedo_single_dataset_paired ) {
    char getFileList1[MAX_ELEMENT_NAME_LENGTH], getFileList2[MAX_ELEMENT_NAME_LENGTH],
        topHat[MAX_ELEMENT_NAME_LENGTH], cufflinks[MAX_ELEMENT_NAME_LENGTH],
        multiplexer[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "get-file-list", MAX_ELEMENT_NAME_LENGTH, getFileList1 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "get-file-list", MAX_ELEMENT_NAME_LENGTH, getFileList2 );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "cufflinks", MAX_ELEMENT_NAME_LENGTH, cufflinks );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "tophat", MAX_ELEMENT_NAME_LENGTH, topHat );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "multiplexer", MAX_ELEMENT_NAME_LENGTH, multiplexer );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, getFileList1, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, getFileList2, "url-in.dataset", "Dataset 1" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, topHat, "out-assembly", cufflinks, "in-assembly" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, getFileList1, "out-url", multiplexer, "input-data-1" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, getFileList2, "out-url", multiplexer, "input-data-2" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, multiplexer, "output-data", topHat, "in-sequence" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, getFileList1, "dataset", topHat,
        "in-sequence.dataset" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, getFileList1, "url", topHat, "in-sequence.in-url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, getFileList2, "url", topHat,
        "in-sequence.paired-url" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, topHat, "hits-url", cufflinks, "in-assembly.url" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/NGS/tuxedo/tuxedo_single_dataset_paired.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, find_sequences ) {
    char readSequence[MAX_ELEMENT_NAME_LENGTH], search[MAX_ELEMENT_NAME_LENGTH],
        writeSequence[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "search", MAX_ELEMENT_NAME_LENGTH, search );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-sequence", MAX_ELEMENT_NAME_LENGTH, writeSequence );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readSequence, "url-in.dataset", "Dataset" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, search, "result-name", "misc_feature" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, search, "use-names", "true" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "document-format", "genbank" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "write-mode", "0" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, search, "out-annotations", writeSequence, "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, readSequence, "out-sequence", search, "in-sequence" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readSequence, "sequence", search,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, search, "annotations", writeSequence, "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", writeSequence,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/Scenarios/find_sequences.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, gene_by_gene_report ) {
    char readSequence[MAX_ELEMENT_NAME_LENGTH], geneByGeneReportId[MAX_ELEMENT_NAME_LENGTH],
        blast[MAX_ELEMENT_NAME_LENGTH], writeSequence[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "genebygene-report-id", MAX_ELEMENT_NAME_LENGTH,
        geneByGeneReportId );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "blast", MAX_ELEMENT_NAME_LENGTH, blast );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-sequence", MAX_ELEMENT_NAME_LENGTH, writeSequence );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readSequence, "url-in.dataset", "Dataset" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "document-format", "genbank" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, blast, "out-annotations", geneByGeneReportId, "in-data" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, readSequence, "out-sequence", blast, "in-sequence" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, blast, "out-annotations", writeSequence, "in-sequence" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readSequence, "sequence", geneByGeneReportId,
        "in-data.gene-seq" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", blast,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", writeSequence,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, blast, "annotations", geneByGeneReportId,
        "in-data.gene-ann" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, blast, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/Scenarios/gene_by_gene_report.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

IMPLEMENT_TEST( CInterfaceManualTests, merge_sequence_annotation ) {
    char readAnnotations[MAX_ELEMENT_NAME_LENGTH], readSequence[MAX_ELEMENT_NAME_LENGTH],
        multiplexer[MAX_ELEMENT_NAME_LENGTH], writeSequence[MAX_ELEMENT_NAME_LENGTH];
    SchemeHandle scheme = NULL;
    U2ErrorType error = createScheme( NULL, &scheme );
    CHECK_U2_ERROR( error );

    error = addElementToScheme( scheme, "read-annotations", MAX_ELEMENT_NAME_LENGTH,
        readAnnotations );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "read-sequence", MAX_ELEMENT_NAME_LENGTH, readSequence );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "multiplexer", MAX_ELEMENT_NAME_LENGTH, multiplexer );
    CHECK_U2_ERROR( error );
    error = addElementToScheme( scheme, "write-sequence", MAX_ELEMENT_NAME_LENGTH, writeSequence );
    CHECK_U2_ERROR( error );

    error = setSchemeElementAttribute( scheme, readSequence, "url-in.dataset", "Dataset" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, readAnnotations, "url-in.dataset", "Dataset" );
    CHECK_U2_ERROR( error );
    error = setSchemeElementAttribute( scheme, writeSequence, "document-format", "genbank" );
    CHECK_U2_ERROR( error );

    error = addFlowToScheme( scheme, readAnnotations, "out-annotations", multiplexer, "input-data-1" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, readSequence, "out-sequence", multiplexer, "input-data-2" );
    CHECK_U2_ERROR( error );
    error = addFlowToScheme( scheme, multiplexer, "output-data", writeSequence, "in-sequence" );
    CHECK_U2_ERROR( error );

    error = addSchemeActorsBinding( scheme, readAnnotations, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "annotations", writeSequence,
        "in-sequence.annotations" );
    CHECK_U2_ERROR( error );
    error = addSchemeActorsBinding( scheme, readSequence, "sequence", writeSequence,
        "in-sequence.sequence" );
    CHECK_U2_ERROR( error );

    U2OpStatusImpl stateInfo;
    SchemeSimilarityUtils::checkSchemesSimilarity( scheme,
        PROPER_WD_SCHEMES_PATH + "/Scenarios/merge_sequence_annotation.uwl", stateInfo );
    CHECK_NO_ERROR( stateInfo );

    releaseScheme( scheme );
}

} // namespace U2