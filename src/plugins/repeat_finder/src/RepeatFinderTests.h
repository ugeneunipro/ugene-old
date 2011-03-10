#ifndef _U2_REPEAT_FINDER_TESTS_H_
#define _U2_REPEAT_FINDER_TESTS_H_

#include "RFBase.h"

#include <U2Core/U2Region.h>

#include <U2Test/XMLTestUtils.h>

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNASequence.h>
#include <U2Core/AppContext.h>
#include <U2Algorithm/BitsTable.h>


#include <QtXml/QDomElement>


namespace U2 {

class FindRepeatsTask;
class TandemFinder;

class GTest_FindSingleSequenceRepeatsTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_FindSingleSequenceRepeatsTask, "find-repeats", TaskFlags_FOSCOE);

    void prepare();
    void run();
    U2Region parseRegion(const QString& n, const QDomElement& el); 

    QString     seq, seq2;
    RFAlgorithm alg;
    
    U2Region     region;
    
    int         w, c, minD, maxD;
    bool        inverted;
    bool        reflect;
    bool        filterNested;
    QStringList excludeList;

    QString     resultFile;

};

class GTest_FindTandemRepeatsTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_FindTandemRepeatsTask, "find-tandems", TaskFlags_FOSCOE);

    void prepare();
    void run();
    U2Region parseRegion(const QString& n, const QDomElement& el); 

    U2Region         region;
    
    char*           string;
    int             minD, maxD;
    int             minSize, maxSize, repeatCount;
    bool            inverted;
    bool            reflect;
    bool            filterNested;
    DNASequence*    seqObj;
    QString         sequence;
    QString         results;

};

class GTest_FindRealTandemRepeatsTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_FindRealTandemRepeatsTask, "find-tandems-real", TaskFlags_FOSCOE);

    void prepare();
    void run();
    U2Region parseRegion(const QString& n, const QDomElement& el);

    U2Region         region;

    char*           string;
    int             minD, maxD;
    int             minSize, repeatCount;
    bool            inverted;
    bool            reflect;
    bool            filterNested;
    DNASequence*    seqObj;
    QString         sequence;
    QString         results;

};

class SArrayIndex;
class SArrayBasedFindTask;

class GTest_SArrayBasedFindTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_SArrayBasedFindTask, "sarray-based-find", TaskFlags_FOSCOE);

    void prepare();
    void run();
        
    QString                 seqObjName;
    QString                 query;
    bool                    useBitMask;
    BitsTable               bt;
    int                     nMismatches;
    DNASequence*            seqObj;
    SArrayIndex*            index;
    SArrayBasedFindTask*    findTask;
    QList<int>              expectedResults;
};

class RepeatFinderTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};


} //namespace
#endif

