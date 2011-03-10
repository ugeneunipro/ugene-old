#ifndef _U2_DNA_EXPORT_PLUGIN_TESTS_H_
#define _U2_DNA_EXPORT_PLUGIN_TESTS_H_


#include <U2Test/XMLTestUtils.h>
#include <QtXml/QDomElement>
#include <U2Core/U2Region.h>
#include "ExportTasks.h"


namespace U2 {

class DNASequenceObject;
class MAlignmentObject;

class GTest_ImportPhredQualityScoresTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_ImportPhredQualityScoresTask, "import-phred-qualities", TaskFlags_NR_FOSCOE);

    void prepare();
    
    QList<DNASequenceObject*> seqList;
    QStringList seqNameList;
    QString     fileName;

};

class GTest_ExportNucleicToAminoAlignmentTask : public GTest {
    Q_OBJECT


public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_ExportNucleicToAminoAlignmentTask, "export-nucleic-alignment", TaskFlags_NR_FOSCOE);

    void prepare();
    ReportResult report();
    QList<Task*> onSubTaskFinished(Task* subTask);

    int                 transTable;
    QString             inputFile;
    QString             outputFileName;
    QString             expectedOutputFile;
    U2Region             selectedRows;
    ExportMSA2MSATask*  exportTask;
    LoadDocumentTask*   resultLoadTask;
    MAlignment          srcAl;
    MAlignment          resAl;
};

class DNAExportPluginTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

} // namespace U2

#endif
