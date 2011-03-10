#ifndef _U2_PRIMER3_TESTS_H_
#define _U2_PRIMER3_TESTS_H_

#include <U2Core/global.h>
#include <U2Test/XMLTestUtils.h>
#include <U2Core/GObject.h>
#include <QtXml/QDomElement>
#include "Primer3Task.h"

namespace U2 {

class GTest_Primer3 : GTest {
Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_Primer3, "plugin_primer_3", TaskFlags(TaskFlag_FailOnSubtaskCancel) | TaskFlag_NoRun);

    ~GTest_Primer3();

    void prepare();
    Task::ReportResult report();

private:
    bool checkPrimerPair(const PrimerPair &primerPair, const PrimerPair &expectedPrimerPair, QString suffix);
    bool checkPrimer(const Primer *primer, const Primer *expectedPrimer, QString prefix, bool internalOligo);
    bool checkIntProperty(int value, int expectedValue, QString name);
    bool checkDoubleProperty(double value, double expectedValue, QString name);
    bool checkAlignProperty(short value, short expectedValue, QString name);
    Primer3SWTask *task;
    Primer3TaskSettings settings;
    QList<PrimerPair> currentBestPairs;
    QList<PrimerPair> expectedBestPairs;
};

}//ns
#endif
