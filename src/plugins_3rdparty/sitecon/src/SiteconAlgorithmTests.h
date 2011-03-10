#ifndef _U2_SITECON_ALG_TEST_H_
#define _U2_SITECON_ALG_TEST_H_

#include "SiteconAlgorithm.h"
#include "SiteconSearchTask.h"
#include <U2Test/XMLTestUtils.h>
#include <U2Core/GObject.h>
#include <U2Core/MAlignment.h>


namespace U2 {

class GTest_CalculateACGTContent : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_CalculateACGTContent, "sitecon-calculateACGTContent", TaskFlags_FOSCOE);

    void prepare();
    void run();
    Task::ReportResult report();
private:
    QString docName;
    SiteconBuildSettings s;
    MAlignment ma;
    int expectedACGT[4];
};

class GTest_CalculateDispersionAndAverage : public GTest {
    Q_OBJECT
    typedef QVector<int> ResultVector;
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_CalculateDispersionAndAverage, "sitecon-calculateDispersionAndAverage", TaskFlags_FOSCOE);

    void prepare();
    void run();
    Task::ReportResult report();
private:
    QString docName;
    SiteconBuildSettings s;
    MAlignment ma;
    QVector<PositionStats> result;
    QVector<ResultVector> expectedResults;    
};

class GTest_CalculateFirstTypeError : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_CalculateFirstTypeError, "sitecon-calculateFirstTypeError", TaskFlags_FOSCOE);

    void prepare();
    void run();
    Task::ReportResult report();
private:
    QString docName;
    int offset;
    SiteconBuildSettings s;
    MAlignment ma;
    QVector<float> result;
    QVector<int> expectedResult;    
};

class GTest_CalculateSecondTypeError : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_CalculateSecondTypeError, "sitecon-calculateSecondTypeError", TaskFlags_FOSCOE);

    void prepare();
    void run();
    Task::ReportResult report();
private:
    QString docName;
    int offset;
    SiteconBuildSettings s;
    MAlignment ma;
    QVector<float> result;
    QVector<int> expectedResult;    
};

class GTest_SiteconSearchTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_SiteconSearchTask, "sitecon-search_task");

    void init();
    void prepare();
    Task::ReportResult report();
private:
    
    SiteconSearchTask *task;
    QString seqName;
    int tresh;
    SiteconModel model;
    bool isNeedCompliment;
    bool complOnly;
    QByteArray seqData;
    QList<SiteconSearchResult>  results;
    QList<SiteconSearchResult>  expectedResults;
};


class GTest_CompareSiteconModels: public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CompareSiteconModels, "compare-sitecon-models");
    Task::ReportResult report();

public:
    QString             doc1ContextName;
    QString             doc2ContextName;

};

}//namespace
#endif
