#ifndef _U2_SW_ALHORITHM_TESTS_H_
#define _U2_SW_ALHORITHM_TESTS_H_

#include <U2Core/GObject.h>
#include <U2Core/U2Region.h>

#include <U2Test/XMLTestUtils.h>

#include <U2Algorithm/SmithWatermanSettings.h>
#include <U2Algorithm/SmithWatermanResult.h>


#include <QtCore/QByteArray>
#include <QtXml/QDomElement>

namespace U2 {

struct SWresult{
        U2Region sInterval;
    int score;
};

class GTest_SmithWatermnan : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_SmithWatermnan, "plugin_sw-algorithm");

    void prepare();
    Task::ReportResult report();

    static void sortByScore(QList<SmithWatermanResult> & resultsForSort);
private:
    SmithWatermanSettings s;
    bool parseExpected_res();
    bool toInt(QString & str, int & num);

    QByteArray patternSeq;
    QByteArray searchSeq;

    QString patternSeqDocName;
    QString searchSeqDocName;
    QString expected_res;
    QString pathToSubst;
    QString impl;
        int gapOpen;
    int gapExtension;
        float percentOfScore;

        QString resultDocContextName;
    QList<SWresult> expectedRes;

    Task * swAlgorithmTask;
    
    QString machinePath;
};

class GTest_SmithWatermnanPerf : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_SmithWatermnanPerf, "test-sw-performance");

    void prepare();
    Task::ReportResult report();

private:
    SmithWatermanSettings s;
    
    QByteArray patternSeq;
    QByteArray searchSeq;

    QString patternSeqDocName;
    QString searchSeqDocName;
    QString pathToSubst;
    QString impl;
    int gapOpen;
    int gapExtension;
    float percentOfScore;

    QString resultDocContextName;
    QList<SWresult> expectedRes;

    Task * swAlgorithmTask;

};


} //namespace
#endif

