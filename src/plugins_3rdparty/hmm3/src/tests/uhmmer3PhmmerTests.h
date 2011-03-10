#ifndef _GB2_UHMMER_PHMMER_TESTS_H_
#define _GB2_UHMMER_PHMMER_TESTS_H_

#include <QtCore/QString>
#include <QtXml/QDomElement>

#include <U2Test/XMLTestUtils.h>
#include <phmmer/uhmm3PhmmerTask.h>

namespace U2 {

/*****************************************
* Test for hmmer3 phmmer.
* settings set by same tags from hmm3-search and hmm3-build tests + gaps probab. options and subst. matr
* we test here 1<->1 queries
*****************************************/
class GTest_UHMM3Phmmer : public GTest {
    Q_OBJECT
public:
    static const QString QUERY_FILENAME_TAG;
    static const QString DB_FILENAME_TAG;
    static const QString PHMMER_TASK_CTX_NAME_TAG;
    
    static const QString GAP_OPEN_PROBAB_OPTION_TAG;
    static const QString GAP_EXTEND_PROBAB_OPTION_TAG;
    static const QString SUBST_MATR_NAME_OPTION_TAG; /* name of registered substitution matrix. if empty - BLOSUM62 is used */
    static const QString SEARCH_CHUNK_OPTION_TAG;
    
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY( GTest_UHMM3Phmmer, "hmm3-phmmer" );
    ~GTest_UHMM3Phmmer();
    
    void prepare();
    ReportResult report();
    void cleanup();
    QList<Task*> onSubTaskFinished(Task* subTask);
    
private:
    void setAndCheckArgs();
    
private:
    UHMM3PhmmerSettings settings;
    QString             queryFilename;
    QString             dbFilename;
    QString             taskCtxName;
    UHMM3SWPhmmerTask * phmmerTask;
    TaskStateInfo       settigsStateInfo;
    TaskStateInfo       scoringSystemStateInfo;
    int                 chunk;
    bool                ctxAdded;
    LoadDocumentTask*   loadDbTask;
    
}; // GTest_UHMM3Phmmer

/*****************************************
* Test compares original hmmer3 phmmer results with UHMM3SearchResults
* 
* Note, that you should make original hmmer3 to show results in academic version (e.g. 1.01e-23)
*****************************************/

class GTest_UHMM3PhmmerCompare : public GTest {
    Q_OBJECT
public:
    static const QString PHMMER_TASK_CTX_NAME_TAG;
    static const QString TRUE_OUT_FILENAME_TAG;

public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY( GTest_UHMM3PhmmerCompare, "hmm3-phmmer-compare" );
    ReportResult report();
    
private:
    void setAndCheckArgs();
    
private:
    QString             phmmerTaskCtxName;
    QString             trueOutFilename;
    UHMM3SWPhmmerTask * phmmerTask;
    
}; // GTest_UHMM3PhmmerCompare

} // U2

#endif // _GB2_UHMMER_PHMMER_TESTS_H_
