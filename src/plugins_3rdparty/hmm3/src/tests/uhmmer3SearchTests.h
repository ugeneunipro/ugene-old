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

#ifndef _GB2_UHMMER3_SEARCH_TESTS_H_
#define _GB2_UHMMER3_SEARCH_TESTS_H_

#include <QtXml/QDomElement>

#include <U2Test/XMLTestUtils.h>
#include <search/uHMM3SearchTask.h>

namespace U2 {


/*****************************************
* Test for hmmer3 search.
* 
* we test here only files with one sequences. no multi-queries
*
* NOTE: you should make hmmer search output files print with 5 signs after point e.g. 0.12345
* NOTE: you should make p7_pli_TargetReportable, p7_pli_DomainReportable report by double, not float (in original hmmer3 code!!!)
*****************************************/
enum GTest_UHMM3SearchAlgoType {
    UNKNOWN_SEARCH = -1,
    GENERAL_SEARCH,
    SEQUENCE_WALKER_SEARCH
};// GTest_UHMM3SearchAlType

class GTest_UHMM3Search : public GTest {
    Q_OBJECT
public:
    static const QString SEQ_DOC_CTX_NAME_TAG; /* loaded sequence document */
    static const QString HMM_FILENAME_TAG;
    static const QString HMMSEARCH_TASK_CTX_NAME_TAG; /* finished UHMM3SearchTask */
    static const QString ALGORITHM_TYPE_OPTION_TAG;
    static const QString SW_CHUNK_SIZE_OPTION_TAG;
    /* reporting thresholds options */
    static const QString SEQ_E_OPTION_TAG; /* -E */
    static const QString SEQ_T_OPTION_TAG; /* -T */
    static const QString Z_OPTION_TAG; /* -Z */
    static const QString DOM_E_OPTION_TAG; /* --domE */
    static const QString DOM_T_OPTION_TAG; /* --domT */
    static const QString DOM_Z_OPTION_TAG; /* --domZ */
    static const QString USE_BIT_CUTOFFS_OPTION_TAG; /* --cut_ga, --cut_nc, --cut_tc or none */
    /* significance thresholds options */
    static const QString INC_SEQ_E_OPTION_TAG; /* --incE */
    static const QString INC_SEQ_T_OPTION_TAG; /* --incT */
    static const QString INC_DOM_E_OPTION_TAG; /* --incdomE */
    static const QString INC_DOM_T_OPTION_TAG; /* --incdomT */
    /* acceleration heuristics options */
    static const QString MAX_OPTION_TAG; /* --max */
    static const QString F1_OPTION_TAG; /* --F1 */
    static const QString F2_OPTION_TAG; /* --F2 */
    static const QString F3_OPTION_TAG; /* --F3 */
    static const QString NOBIAS_OPTION_TAG; /* --nobias */
    static const QString NONULL2_OPTION_TAG; /* --nonull2 */
    static const QString SEED_OPTION_TAG; /* --seed */
    
    static const QString REMOTE_MACHINE_VAR;
    
    static void setSearchTaskSettings( UHMM3SearchSettings& set, const QDomElement& el, TaskStateInfo& si );
    
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY( GTest_UHMM3Search, "hmm3-search" );
    ~GTest_UHMM3Search();
    
    void prepare();
    ReportResult report();
    void cleanup();
    
    virtual QList< Task* > onSubTaskFinished( Task * sub );
    
private:
    void setAndCheckArgs();

private:
    UHMM3SearchTaskSettings settings;
    QString                 hmmFilename;
    Task*                   searchTaskToCtx; /* general or sw or RemoteTask. we will add it to context */
    QString                 searchTaskCtxName;
    DNASequence             sequence;
    QString                 seqDocCtxName;
    GTest_UHMM3SearchAlgoType algo;
    int                     swChunk;
    bool                    cleanuped;
    
    QString                 machinePath;
    P7_HMM *                hmm;
    bool                    ctxAdded;
    
}; // GTest_GeneralUHMM3Search

/*****************************************
* Test compares original hmmer3 search results with UHMM3SearchResults
* 
* Note, that you should make original hmmer3 to show results in academic version (e.g. 1.01e-23)
*****************************************/
class GTest_UHMM3SearchCompare : public GTest {
    Q_OBJECT
public:
    static const QString SEARCH_TASK_CTX_NAME_TAG;
    static const QString TRUE_OUT_FILE_TAG; /* file with original hmmer3 output */
    
    static UHMM3SearchResult getOriginalSearchResult( const QString & filename );
    static void generalCompareResults( const UHMM3SearchResult& myRes, const UHMM3SearchResult& trueRes, TaskStateInfo& ti );
    static void swCompareResults( const QList<UHMM3SWSearchTaskDomainResult>& myR, const UHMM3SearchResult& trueR, 
        TaskStateInfo& ti, bool compareSeqRegion = false );
    static bool searchResultLessThan(const UHMM3SearchSeqDomainResult & r1, const UHMM3SearchSeqDomainResult & r2);
    
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY( GTest_UHMM3SearchCompare, "hmm3-search-compare" );
    ReportResult report();

private:
    void setAndCheckArgs();

private:
    QString                     searchTaskCtxName;
    QString                     trueOutFilename;
    UHMM3LoadProfileAndSearchTask *generalTask;
    UHMM3SWSearchTask*          swTask;
    GTest_UHMM3SearchAlgoType   algo;

}; // GTest_GeneralUHMM3SearchCompare

}

#endif // _GB2_UHMMER3_SEARCH_TESTS_H_
