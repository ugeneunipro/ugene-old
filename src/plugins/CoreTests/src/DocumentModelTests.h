#ifndef _U2_DOCUMENT_MODEL_TESTS_H_
#define _U2_DOCUMENT_MODEL_TESTS_H_

#include <U2Test/XMLTestUtils.h>

#include <U2Core/IOAdapter.h>

#include <QtXml/QDomElement>

namespace U2 {

class Document;
class GObject;
class LoadDocumentTask;
class SaveDocumentTask;

class GTest_LoadDocument : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_LoadDocument, "load-document");

    ReportResult report();

    virtual void cleanup();

private:
    QString             docContextName;
    LoadDocumentTask*   loadTask;
    bool                contextAdded;
    bool                tempFile;
    QString             url;
};

class GTest_SaveDocument : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_SaveDocument, "save-document");
    void prepare();

private:
     QString             url;
     IOAdapterFactory*   iof;


    QString             docContextName;
    SaveDocumentTask*   saveTask;
};

class GTest_LoadBrokenDocument : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_LoadBrokenDocument, "load-broken-document", TaskFlags(TaskFlag_NoRun)| TaskFlag_FailOnSubtaskCancel);

    Document* getDocument() const;
    ReportResult report();

private:
    LoadDocumentTask*   loadTask;
};

class GTest_DocumentNumObjects : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_DocumentNumObjects, "check-num-objects");

    ReportResult report();
    QString docContextName;
    int numObjs;
};

class GTest_DocumentObjectNames : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_DocumentObjectNames, "check-document-object-names");

    ReportResult report();
    
    QString docContextName;
    QStringList names;
};

class GTest_DocumentObjectTypes : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_DocumentObjectTypes, "check-document-object-types");

    ReportResult report();
    
    QString docContextName;
    QList<GObjectType> types;
};

class DocumentModelTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};


class GTest_FindGObjectByName : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_FindGObjectByName, "find-object-by-name");

    ReportResult report();

    void cleanup();

private:
    QString     docContextName;
    QString     objContextName;
    QString     objName;
    GObjectType type;
    GObject*    result;
    
};

class GTest_CompareFiles : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CompareFiles, "compare-docs");
    
    ReportResult report();
    
private:
    QString doc1Path;
    QString doc2Path;
    bool byLines;
    
};

}//namespace
#endif
