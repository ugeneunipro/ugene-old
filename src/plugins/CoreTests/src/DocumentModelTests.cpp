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

#include "DocumentModelTests.h"

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>
#include <U2Core/GHints.h>
#include <U2Core/Log.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/DocumentUtils.h>

#include <U2Test/GTest.h>

#include <U2Formats/SAMFormat.h>


namespace U2 {

/* TRANSLATOR U2::GTest */

#define VALUE_ATTR "value"
#define DOC_ATTR   "doc"
#define NAME_ATTR   "name"
#define TYPE_ATTR   "type"

/*******************************
 * GTest_LoadDocument
 *******************************/
static QString getTempDir(const GTestEnvironment* env) {
    QString result = env->getVar("TEMP_DATA_DIR");
    if (result.isEmpty()) {
        result = QCoreApplication::applicationDirPath();
    }
    return result;
}

void GTest_LoadDocument::init(XMLTestFormat*, const QDomElement& el) {
    loadTask = NULL;
    contextAdded = false;
    docContextName = el.attribute("index");
    needVerifyLog = false;

    if (NULL != el.attribute("message")) {
        expectedLogMessage = el.attribute("message");
    }

    if (NULL != el.attribute("message2")) {
        expectedLogMessage2 = el.attribute("message2");
    }

    if (NULL != el.attribute("no-message")) {
        unexpectedLogMessage = el.attribute("no-message");
    }
    
    QString dir = el.attribute("dir");
    if(dir == "temp"){
        tempFile = true;
        url = getTempDir(env) + "/" + el.attribute("url");   
    } else{
        tempFile = false;
        QString commonDataDir = env->getVar("COMMON_DATA_DIR");
        url =  commonDataDir + "/" + el.attribute("url");
    }
    IOAdapterId         io = el.attribute("io");
    IOAdapterFactory*   iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(io);
    DocumentFormatId    format = el.attribute("format");
    QVariantMap         fs;
    if (iof == NULL) {
        stateInfo.setError( QString("io_adapter_not_found_%1").arg(io));
    } else if (format.isEmpty()) {
        stateInfo.setError( QString("doc_format_is_not_specified"));
    } else {
        if (format == BaseDocumentFormats::SAM) {
            //SAM format is temporarily removed from base formats list -> create it manually
            SAMFormat* samFormat = new SAMFormat();
            loadTask = new LoadDocumentTask(samFormat, url, iof);
            samFormat->setParent(loadTask);
        } else {
            loadTask = new LoadDocumentTask(format, url, iof);
        }
        addSubTask(loadTask);
    }
}

void GTest_LoadDocument::cleanup() {
    if (contextAdded) {
        removeContext(docContextName);
    }
    if(tempFile){
        QFile::remove(url);
    }
}

void GTest_LoadDocument::prepare() {
    QStringList expectedMessages;
    QStringList unexpectedMessages;

    if (!expectedLogMessage.isEmpty()) {
        expectedMessages << expectedLogMessage;
    }

    if (!expectedLogMessage2.isEmpty()) {
        expectedMessages << expectedLogMessage2;
    }

    if (!unexpectedLogMessage.isEmpty()){
        unexpectedMessages << unexpectedLogMessage;
    }
    if(expectedLogMessage.length()!=0 || unexpectedMessages.length()!=0){
        needVerifyLog=true;
        logHelper.initMessages(expectedMessages, unexpectedMessages);
    }
}

Task::ReportResult GTest_LoadDocument::report() {
    if (loadTask!=NULL && loadTask->hasError()) {
        stateInfo.setError( loadTask->getError());
    } else if (!docContextName.isEmpty()) {
        addContext(docContextName, loadTask->getDocument());
        contextAdded = true;

        if (needVerifyLog && GTest_LogHelper_Invalid == logHelper.verifyStatus()) {
            stateInfo.setError("Log is incorrect!");
        }
    }
    return ReportResult_Finished;
}


/*******************************
* GTest_SaveDocument
*******************************/
void GTest_SaveDocument::init(XMLTestFormat* tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    saveTask = NULL;
    QString dir = getTempDir(env);
    if (!QDir(dir).exists()) {
        bool ok = QDir::root().mkpath(dir);
        if (!ok) {
            setError(QString("Can't create TEMP_DATA_DIR dir: %1").arg(dir));
            return;
        }
    }

    url = dir + "/" + el.attribute("url");
    
    IOAdapterId io = el.attribute("io");
    iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(io);

    if (iof == NULL) {
        stateInfo.setError(QString("io_adapter_not_found_%1").arg(io));
        return ;
    } 

}

void GTest_SaveDocument::prepare(){
    //////////////////////
    Document* doc = getContext<Document>(this, docContextName);
    if (doc == NULL) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ;
    }
    saveTask = new SaveDocumentTask(doc, iof, url);
    addSubTask(saveTask);
    /////////////////////////
}

/*******************************
* GTest_LoadBrokenDocument
*******************************/
void GTest_LoadBrokenDocument::init(XMLTestFormat* tf, const QDomElement& el) {
    Q_UNUSED(tf);

    QString             urlAttr = el.attribute("url");
    QString             dir = el.attribute("dir");
    IOAdapterId         io = el.attribute("io");
    IOAdapterFactory*   iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(io);
    DocumentFormatId    format = el.attribute("format");
    QVariantMap         fs;

    tempFile = (dir == "temp");

    if (dir == "temp") {
        url = getTempDir(env) + "/" + urlAttr;
    } else {
        url = env->getVar("COMMON_DATA_DIR") + "/" + urlAttr;
    }

    message = el.attribute("message");

    loadTask = new LoadDocumentTask(format, url, iof);
    addSubTask(loadTask);
}

Task::ReportResult GTest_LoadBrokenDocument::report() {
    Document* doc = loadTask->getDocument();
    if (doc == NULL && loadTask->hasError()) {
        CHECK(!message.isEmpty(), ReportResult_Finished);
        if (loadTask->getError().contains(message)) {
            return ReportResult_Finished;
        }
        stateInfo.setError(QString("expected message is not found"));
        return ReportResult_Finished;
    }
    stateInfo.setError(QString("file read without errors"));
    return ReportResult_Finished;
}

void GTest_LoadBrokenDocument::cleanup() {
    if (tempFile) {
        QFile::remove(url);
    }
}

/*******************************
*GTest_DocumentFormat
*******************************/

void GTest_DocumentFormat::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docUrl = el.attribute("url");
    if (docUrl.isEmpty()) {
        failMissingValue("url");
        return;
    }
    docUrl = env->getVar("COMMON_DATA_DIR") + "/" + docUrl;

    docFormat = el.attribute(VALUE_ATTR);
    if (docFormat.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    } 
} 

Task::ReportResult GTest_DocumentFormat::report() {
    
    U2OpStatus2Log opStatus;
    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(GUrl(docUrl), opStatus);
    if(formats.isEmpty()) {
        stateInfo.setError(QString("Can't detect format for file %1").arg(docUrl));
        return ReportResult_Finished;
    } 

    QString format = formats.first().format->getFormatId();
    if (format != docFormat) {
        stateInfo.setError(QString("Format not matched: %1, expected %2").arg(format).arg(docFormat));
    }
    return ReportResult_Finished;
}

/*******************************
* GTest_DocumentNumObjects
*******************************/
void GTest_DocumentNumObjects::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    QString v = el.attribute(VALUE_ATTR);
    if (v.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    } 
    bool ok = false;
    numObjs = v.toInt(&ok);
    if (!ok) {
        failMissingValue(VALUE_ATTR);
    }
}

Task::ReportResult GTest_DocumentNumObjects::report() {
    Document* doc = getContext<Document>(this, docContextName);
    if (doc == NULL) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ReportResult_Finished;
    }
    int num = doc->getObjects().size();
    if (num != numObjs) {
        stateInfo.setError(QString("number of objects in document not matched: %1, expected %2").arg(num).arg(numObjs));
    }
    return ReportResult_Finished;
}

/*******************************
* GTest_DocumentObjectNames
*******************************/
void GTest_DocumentObjectNames::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    QString v = el.attribute(VALUE_ATTR);
    if (v.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    }
    names =  v.split(",");
}


Task::ReportResult GTest_DocumentObjectNames::report() {
    Document* doc = getContext<Document>(this, docContextName);
    if (doc == NULL) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ReportResult_Finished;
    }
    const QList<GObject*>& objs = doc->getObjects();
    int namesSize = names.size();
    int objsSize = objs.size();
    if (namesSize != objsSize) {
        stateInfo.setError(QString("number of objects in document not matched: %1, expected %2").arg(objs.size()).arg(names.size()));
        return ReportResult_Finished;
    }
    for(int i=0; i<names.size(); i++) {
        QString name = names[i];
        QString objName = objs[i]->getGObjectName();
        if (name!=objName) {
            stateInfo.setError(QString("name of the objects not matched: %1, expected %2").arg(objName).arg(name));
            return ReportResult_Finished;
        }
    }
    return ReportResult_Finished;
}

/*******************************
* GTest_DocumentObjectTypes
*******************************/
void GTest_DocumentObjectTypes::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    QString v = el.attribute(VALUE_ATTR);
    if (v.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    }
    types =  v.split(",");
}

Task::ReportResult GTest_DocumentObjectTypes::report() {
    const Document* doc = getContext<Document>(this, docContextName);
    if (doc == NULL) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ReportResult_Finished;
    }
    const QList<GObject*>& objs = doc->getObjects();
    int typesSize = types.size();
    int objsSize = objs.size();
    if (typesSize != objsSize) {
        stateInfo.setError(QString("number of objects in document not matched: %1, expected %2").arg(objs.size()).arg(types.size()));
        return ReportResult_Finished;
    }
    for(int i=0; i<types.size(); i++) {
        GObjectType type = types[i];
        GObjectType objType = objs[i]->getGObjectType();
        if (type!=objType) {
            stateInfo.setError(QString("types of the objects not matched: %1, expected %2").arg(objType).arg(type));
            return ReportResult_Finished;
        }
    }
    return ReportResult_Finished;
}

/*******************************
* GTest_FindGObjectByName
*******************************/
void GTest_FindGObjectByName::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    objName = el.attribute(NAME_ATTR);
    if (objName.isEmpty()) {
        failMissingValue(NAME_ATTR);
        return;
    }

    type = el.attribute(TYPE_ATTR);
    if (type.isEmpty()) {
        failMissingValue(TYPE_ATTR);
        return;
    }

    objContextName = el.attribute("index");

    result = NULL;

}

Task::ReportResult GTest_FindGObjectByName::report() {
    const Document* doc = getContext<Document>(this, docContextName);
    if (doc == NULL) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ReportResult_Finished;
    }
    const QList<GObject*>& objs = doc->getObjects();

    foreach(GObject* obj, objs) {    
        if ((obj->getGObjectType() == type) && (obj->getGObjectName() == objName)) {
            result = obj;
            break;
        }
    }
    if (result == NULL) {
        stateInfo.setError(QString("object not found: name '%1',type '%2' ").arg(objName).arg(type));
    } else if (!objContextName.isEmpty()) {
        addContext(objContextName, result);
    }
    return ReportResult_Finished;
}


void GTest_FindGObjectByName::cleanup() {
    if (result!=NULL && !objContextName.isEmpty()) {
        removeContext(objContextName);
    }
}


/*******************************
 * GTest_CompareFiles
 *******************************/

static const QString DOC1_ATTR_ID = "doc1";
static const QString DOC2_ATTR_ID = "doc2";
static const QString TMP_ATTR_ID  = "temp";
static const QString TMP_ATTR_SPLITTER = ",";
static const QString BY_LINES_ATTR_ID = "by_lines";
static const QString COMMENTS_START_WITH = "comments_start_with";

void GTest_CompareFiles::replacePrefix(QString &path) {
    QString result;

    const QString EXPECTED_OUTPUT_DIR_PREFIX = "!expected!";
    const QString TMP_DATA_DIR_PREFIX = "!tmp_data_dir!";

    // Determine which environment variable is required
    QString envVarName;
    QString prefix;
    if (path.startsWith(EXPECTED_OUTPUT_DIR_PREFIX)) {
        envVarName = "EXPECTED_OUTPUT_DIR";
        prefix = EXPECTED_OUTPUT_DIR_PREFIX;
    }
    else if (path.startsWith(TMP_DATA_DIR_PREFIX)) {
        envVarName = "TEMP_DATA_DIR";
        prefix = TMP_DATA_DIR_PREFIX;
    }
    else {
        FAIL(QString("Unexpected 'prefix' value in the path: '%1'!").arg(path), );
    }

    // Replace with the correct value
    QString prefixPath = env->getVar(envVarName);
    SAFE_POINT(!prefixPath.isEmpty(), QString("No value for environment variable '%1'!").arg(envVarName), );
    prefixPath += "/";

    int prefixSize = prefix.size();
    QStringList relativePaths = path.mid(prefixSize).split(";");

    foreach (const QString &path, relativePaths) {
        QString fullPath = prefixPath + path;
        result += fullPath + ";";
    }

    path = result.mid(0, result.size() - 1); // without the last ';'
}

void GTest_CompareFiles::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    // Get the attributes values
    QString tmpAttr = el.attribute(TMP_ATTR_ID);

    doc1Path = el.attribute(DOC1_ATTR_ID);
    if(doc1Path.isEmpty()) {
        failMissingValue(DOC1_ATTR_ID);
        return;
    }

    doc2Path = el.attribute(DOC2_ATTR_ID);
    if(doc2Path.isEmpty()) {
        failMissingValue(DOC2_ATTR_ID);
        return;
    }

    // Get the full documents paths
    if (!tmpAttr.isEmpty()) {
        // Attribute "tmp" is used to determine paths prefixes,
        // specified paths are relative (used in all old tests, obsolete)
        QStringList tmpDocNums = tmpAttr.split(TMP_ATTR_SPLITTER, QString::SkipEmptyParts);
        doc1Path = (tmpDocNums.contains("1") ? env->getVar("TEMP_DATA_DIR") : env->getVar("COMMON_DATA_DIR")) + "/" + doc1Path;
        doc2Path = (tmpDocNums.contains("2") ? env->getVar("TEMP_DATA_DIR") : env->getVar("COMMON_DATA_DIR")) + "/" + doc2Path;
        byLines = !el.attribute(BY_LINES_ATTR_ID).isEmpty();
        if(el.attribute(COMMENTS_START_WITH).isEmpty()){
            commentsStartWith=QStringList();
        }else{
            QString commentsStartWithString=el.attribute(COMMENTS_START_WITH);
            commentsStartWith=commentsStartWithString.split(",");
        }
    }
    else {
        // Only "doc1" and "doc2" attributes are specified,
        // paths contain prefixes, e.g. "!common_data_dir!"
        replacePrefix(doc1Path);
        replacePrefix(doc2Path);
    }
}

static const qint64 READ_LINE_MAX_SZ = 2048;

Task::ReportResult GTest_CompareFiles::report() {
    QFile f1(doc1Path);
    if(!f1.open(QIODevice::ReadOnly)) {
        setError(QString("Cannot open file '%1'!").arg(doc1Path));
        return ReportResult_Finished;
    }
    
    QFile f2(doc2Path);
    if(!f2.open(QIODevice::ReadOnly)) {
        setError(QString("Cannot open file '%1'!").arg(doc2Path));
        return ReportResult_Finished;
    }
    
    int lineNum = 0;
    while(1) {
        QByteArray bytes1 = f1.readLine(READ_LINE_MAX_SZ);
        QByteArray bytes2 = f2.readLine(READ_LINE_MAX_SZ);
        
        if(bytes1.isEmpty() || bytes2.isEmpty()) {
            if( bytes1 != bytes2 ) {
                setError(QString("The files are of different sizes!"));
                return ReportResult_Finished;
            }
            break;
        }
        
        if(byLines) {
            bytes1 = bytes1.trimmed();
            bytes2 = bytes2.trimmed();
        }
        if(commentsStartWith.isEmpty()){
            if( bytes1 != bytes2 ) {
                setError(QString("The files are not equal at line %1."
                    "The first file contains '%2'' and the second contains '%3'!")
                    .arg(lineNum).arg(QString(bytes1)).arg(QString(bytes2)));
                return ReportResult_Finished;
            }
        }else{
            foreach (QString commentStartWith, commentsStartWith) {
                if(!bytes1.startsWith(commentStartWith.toLatin1()) && !bytes2.startsWith(commentStartWith.toLatin1())){
                    if( bytes1 != bytes2 ) {
                        setError(QString("The files are not equal at line %1."
                            "The first file contains '%2'' and the second contains '%3'!")
                            .arg(lineNum).arg(QString(bytes1)).arg(QString(bytes2)));
                        return ReportResult_Finished;
                    }
                }else
                if(!(bytes1.startsWith(commentStartWith.toLatin1())&&bytes2.startsWith(commentStartWith.toLatin1()))){
                    setError(QString("The files have comments and are not equal at line %1."
                        "The first file contains '%2'' and the second contains '%3'!")
                        .arg(lineNum).arg(QString(bytes1)).arg(QString(bytes2)));
                    return ReportResult_Finished;
                }

            }
        }
        
        if(bytes1.endsWith("\n") || byLines) {
            lineNum++;
        }
    }
    
    return ReportResult_Finished;
}

/*******************************
 * GTest_Compare_VCF_Files
 *******************************/
const int NUMBER_OF_COLUMNS = 5;
void GTest_Compare_VCF_Files::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    QStringList tmpDocNums = el.attribute(TMP_ATTR_ID).split(TMP_ATTR_SPLITTER, QString::SkipEmptyParts);

    doc1Path = el.attribute(DOC1_ATTR_ID);
    if(doc1Path.isEmpty()) {
        failMissingValue(DOC1_ATTR_ID);
        return;
    }
    doc1Path = (tmpDocNums.contains("1") ? env->getVar( "TEMP_DATA_DIR" ) : env->getVar("COMMON_DATA_DIR")) + "/" + doc1Path;

    doc2Path = el.attribute(DOC2_ATTR_ID);
    if(doc2Path.isEmpty()) {
        failMissingValue(DOC2_ATTR_ID);
        return;
    }
    doc2Path = (tmpDocNums.contains("2") ? env->getVar( "TEMP_DATA_DIR" ) : env->getVar("COMMON_DATA_DIR")) + "/" + doc2Path;

    byLines = !el.attribute(BY_LINES_ATTR_ID).isEmpty();
}


Task::ReportResult GTest_Compare_VCF_Files::report() {
    QFile f1(doc1Path);
    if(!f1.open(QIODevice::ReadOnly)) {
        setError(QString("Cannot open %1 file").arg(doc1Path));
        return ReportResult_Finished;
    }

    QFile f2(doc2Path);
    if(!f2.open(QIODevice::ReadOnly)) {
        setError(QString("Cannot open %1 file").arg(doc2Path));
        return ReportResult_Finished;
    }

    int lineNum = 0;
    while(1) {
        QByteArray bytes1 = f1.readLine(READ_LINE_MAX_SZ);

        QByteArray bytes2 = f2.readLine(READ_LINE_MAX_SZ);

        if(bytes1.isEmpty() || bytes2.isEmpty()) {
            if( bytes1 != bytes2 ) {
                setError(QString("files are of different size"));
                return ReportResult_Finished;
            }
            break;
        }
        int size = qMin<int>(bytes1.size(), bytes2.size());

        int tabCounter=0;
        for(int i=0; i<size; i++){
            if (bytes1[i]=='\t'){
                tabCounter++;
                if(tabCounter==NUMBER_OF_COLUMNS){
                    break;
                }
            }

            if( bytes1[i] != bytes2[i] ) {
                setError(QString("files are note equal at line %1. %2 and %3").arg(lineNum).arg(QString(bytes1)).arg(QString(bytes2)));
                return ReportResult_Finished;
            }
        }



        if(bytes2.endsWith("\n") || byLines) {
            lineNum++;
        }
    }

    return ReportResult_Finished;
}

/*******************************
 * GTest_PDF_CompareFiles
 *******************************/
const int NUMDER_OF_LINES = 10;

void GTest_Compare_PDF_Files::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    QStringList tmpDocNums = el.attribute(TMP_ATTR_ID).split(TMP_ATTR_SPLITTER, QString::SkipEmptyParts);

    doc1Path = el.attribute(DOC1_ATTR_ID);
    if(doc1Path.isEmpty()) {
        failMissingValue(DOC1_ATTR_ID);
        return;
    }
    doc1Path = (tmpDocNums.contains("1") ? env->getVar( "TEMP_DATA_DIR" ) : env->getVar("COMMON_DATA_DIR")) + "/" + doc1Path;

    doc2Path = el.attribute(DOC2_ATTR_ID);
    if(doc2Path.isEmpty()) {
        failMissingValue(DOC2_ATTR_ID);
        return;
    }
    doc2Path = (tmpDocNums.contains("2") ? env->getVar( "TEMP_DATA_DIR" ) : env->getVar("COMMON_DATA_DIR")) + "/" + doc2Path;

    byLines = !el.attribute(BY_LINES_ATTR_ID).isEmpty();
}

Task::ReportResult GTest_Compare_PDF_Files::report() {
    QFile f1(doc1Path);
    if(!f1.open(QIODevice::ReadOnly)) {
        setError(QString("Cannot open %1 file").arg(doc1Path));
        return ReportResult_Finished;
    }

    QFile f2(doc2Path);
    if(!f2.open(QIODevice::ReadOnly)) {
        setError(QString("Cannot open %1 file").arg(doc2Path));
        return ReportResult_Finished;
    }

    int lineNum = 0;
    int i=0;
    while(1) {
        QByteArray bytes1 = f1.readLine(READ_LINE_MAX_SZ);
        QByteArray bytes2 = f2.readLine(READ_LINE_MAX_SZ);

        if(i<NUMDER_OF_LINES){
            i++;
            continue;
        }//skip first lines containing file info

        if(bytes1.isEmpty() || bytes2.isEmpty()) {
            if( bytes1 != bytes2 ) {
                setError(QString("files are of different size"));
                return ReportResult_Finished;
            }
            break;
        }

        if(byLines) {
            bytes1 = bytes1.trimmed();
            bytes2 = bytes2.trimmed();
        }

        if( bytes1 != bytes2 ) {
            setError(QString("files are note equal at line %1. %2 and %3").arg(lineNum).arg(QString(bytes1)).arg(QString(bytes2)));
            return ReportResult_Finished;
        }

        if(bytes1.endsWith("\n") || byLines) {
            lineNum++;
        }
    }

    return ReportResult_Finished;
}



/*******************************
* DocumentModelTests
*******************************/
QList<XMLTestFactory*> DocumentModelTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_LoadDocument::createFactory());
    res.append(GTest_LoadBrokenDocument::createFactory());
    res.append(GTest_DocumentFormat::createFactory());
    res.append(GTest_DocumentNumObjects::createFactory());
    res.append(GTest_DocumentObjectNames::createFactory());
    res.append(GTest_DocumentObjectTypes::createFactory());
    res.append(GTest_FindGObjectByName::createFactory());
    res.append(GTest_SaveDocument::createFactory());
    res.append(GTest_CompareFiles::createFactory());
    res.append(GTest_Compare_VCF_Files::createFactory());
    res.append(GTest_Compare_PDF_Files::createFactory());
    return res;
}


} // namespace
