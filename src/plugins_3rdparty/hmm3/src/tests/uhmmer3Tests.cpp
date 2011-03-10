#include "uhmmer3BuildTests.h"
#include "uhmmer3SearchTests.h"
#include "uhmmer3PhmmerTests.h"

#include "uhmmer3Tests.h"

namespace U2 {

QList< XMLTestFactory* > UHMMER3Tests::createTestFactories() {
    QList< XMLTestFactory* > res;
    res << GTest_CompareHmmFiles::createFactory();
    res << GTest_UHMMER3Build::createFactory();
    res << GTest_UHMM3Search::createFactory();
    res << GTest_UHMM3SearchCompare::createFactory();
    res << GTest_UHMM3Phmmer::createFactory();
    res << GTest_UHMM3PhmmerCompare::createFactory();
    return res;
}

} // GB2
