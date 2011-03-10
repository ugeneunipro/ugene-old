#ifndef _U2_DNA_ASSEMBLY_TESTS_H_
#define _U2_DNA_ASSEMBLY_TESTS_H_

#include <QtCore/QList>

#include <U2Test/XMLTestUtils.h>
#include <U2Core/GUrl.h>

namespace U2 {

class DnaAssemblyMultiTask;
class MAlignmentObject;
class DNASequenceObject;

class GTest_DnaAssemblyToReferenceTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_DnaAssemblyToReferenceTask, "test-dna-assembly-to-ref");
    void prepare();
    Task::ReportResult report();
    void cleanup();
private:
    DnaAssemblyMultiTask* assemblyMultiTask;
    QList<GUrl> shortReadUrls;
    QMap<QString,QString> customOptions;
    QString objName;
    MAlignmentObject* expectedObj;
    QString algName;
    QString refSeqUrl;
    QString indexFileName;
    GUrl resultFileName;
    bool newIndex;
};


class DnaAssemblyTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

} //namespace
#endif
