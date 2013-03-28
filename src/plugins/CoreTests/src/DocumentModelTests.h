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
    void prepare();

    virtual void cleanup();


private:
    QString             docContextName;
    LoadDocumentTask*   loadTask;
    bool                contextAdded;
    bool                tempFile;
    QString             url;
    GTestLogHelper      logHelper;

    QString expectedLogMessage;
    QString expectedLogMessage2;
    QString unexpectedLogMessage;

    bool                needVerifyLog;
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
    void cleanup();

private:
    LoadDocumentTask*   loadTask;
    QString url;
    bool tempFile;
};

class GTest_DocumentNumObjects : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_DocumentNumObjects, "check-num-objects");

    ReportResult report();
    QString docContextName;
    int numObjs;
};

class GTest_DocumentFormat : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_DocumentFormat, "check-document-format");

    ReportResult report();

    QString docUrl;
    QString docFormat;
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

class GTest_Compare_VCF_Files : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_Compare_VCF_Files, "compare-vcf-docs");

    ReportResult report();

private:
    QString doc1Path;
    QString doc2Path;
    bool byLines;

};

class GTest_Compare_PDF_Files : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_Compare_PDF_Files, "compare-pdf-docs");

    ReportResult report();

private:
    QString doc1Path;
    QString doc2Path;
    bool byLines;

};

}//namespace
#endif
