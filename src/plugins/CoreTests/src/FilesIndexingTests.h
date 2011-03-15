/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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


#ifndef _U2_FILES_INDEXING_TESTS_H_
#define _U2_FILES_INDEXING_TESTS_H_

#include <QtCore/QTemporaryFile>

#include <U2Test/XMLTestUtils.h>

#include <U2Core/GetDocumentFromIndexTask.h>
#include <U2Core/CreateFileIndexTask.h>

namespace U2 {

/*
    creates an index of list of files
    input is a list of files in COMMON_DATA_DIR directory if TMP_INPUT_TAG is not set, otherwise
        input files are looked in context as temporary files ( QTemporaryFile )
    output file is created in TEMP_DATA_DIR and deleted if DELETE_OUTPUT_TAG is set( not empty )
*/
class GTest_CreateFileIndex : public GTest {
    Q_OBJECT
public:
    static const QString INPUT_URLS_TAG;
    static const QString TMP_INPUT_TAG;
    static const QString INPUT_URLS_SEPARATOR;
    static const QString OUTPUT_URL_TAG;
    static const QString DELETE_OUTPUT_TAG;

public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CreateFileIndex, "create-file-index" );
    ~GTest_CreateFileIndex();
    
    void prepare();
    ReportResult report();
    
private:
    CreateFileIndexTask* createIndTask;
    bool                 delOutput;
    bool                 tmpInput;
    
    QStringList                inputUrls;
    QList< IOAdapterFactory* > inFactories;
    QString                    outputUrl;
    IOAdapterFactory*          outFactory;
    
}; // GTest_CreateFileIndex

/*
    generates a file from a list of files
    input is a list of files in COMMON_DATA_DIR directory and a list of numbers.
    output file is created in TEMP_DATA_DIR and QTemporary file can be found in context
    output is gzipped if GZIP_OUTPUT_TAG is set
*/
class GTest_GenerateFileTest : public GTest {
    Q_OBJECT
public:
    static const QString INPUT_URLS_TAG;
    static const QString INPUT_NUMBERS_TAG;
    static const QString INPUT_SEPARATOR;
    
    static const QString OUTPUT_CTX_NAME_TAG;
    static const QString GZIP_OUTPUT_TAG;
    
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_GenerateFileTest, "generate-file" );
    ~GTest_GenerateFileTest();
    
    virtual void run();
    void prepare();
    ReportResult report();
    void cleanup();
    
private:
    QTemporaryFile* outFile;
    bool            ctxAdded;
    QString         outCtxName;
    QString         outFilenameTemplate;
    
    QStringList     inputUrls;
    QList< int >    numbers;
    
};

/*
    loads a document using a index file and number of document in that index
    input: Document object context name is given in INDEX_DOCUMENT_TAG
           document must contain one UindexObject object
    output: document is added to context
*/
class GTest_GetDocFromIndexTest : public GTest {
    Q_OBJECT
public:
    static const QString INDEX_DOCUMENT_TAG;
    static const QString DOC_NUMBER_TAG;
    static const QString DOC_CTX_NAME_TAG;
    
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY( GTest_GetDocFromIndexTest, "get-doc-from-index" );
    ~GTest_GetDocFromIndexTest();
    
    void prepare();
    ReportResult report();
    void cleanup();
    
private:
    QString indDocName;
    int     docNum;
    QString docCtxName;
    bool    ctxAdded;
    
    GetDocumentFromIndexTask* getDocTask;
    
};

class FilesIndexingTests {
public:
    static QList< XMLTestFactory* > createTestFactories();
    
}; // FilesIndexingTests

} // U2

#endif // _U2_FILES_INDEXING_TESTS_H_
