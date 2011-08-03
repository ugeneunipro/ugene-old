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


#include <U2Core/AppContext.h>

#include <U2Core/UIndexObject.h>
#include <U2Core/IOAdapterUtils.h>

#include "FilesIndexingTests.h"

namespace {

const QString ENV_COMMON_DATA_DIR = "COMMON_DATA_DIR";
const QString ENV_TMP_DATA_DIR    = "TEMP_DATA_DIR";

const QString UGENE_TMP_FILE_TEMPLATE = "ugene_tmpXXXXXX";
const QString GZIP_END                = ".gz";

const int BUF_SZ   = 16384;
const int READ_ERR = -1;

const QString NO_ANSWER = "no";
const QString N_ANSWER  = "n";

using namespace U2;

void addStr( QStringList& l, const QString& add ) {
    QStringList::iterator it = l.begin();
    while( l.end() != it ) {
        *it = add + ( *it );
        ++it;
    }
}

QList< IOAdapterFactory* > getIOAdapterFactories( const QStringList& urls ) {
    QList< IOAdapterFactory* > ret;
    foreach( QString url, urls ) {
        ret << AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( IOAdapterUtils::url2io( url ) );
    }
    return ret;
}

QList< int > getNumbers( const QStringList& numList, bool& ok ) {
    QList< int > nums;
    
    foreach( const QString& str, numList ) {
        bool curOk = false;
        int cur = str.toInt( &curOk );
        if( !curOk || 0 > cur ) {
            ok = false;
            return nums;
        }
        nums.append( cur );
    }
    ok = true;
    return nums;
}

IOAdapter* getOpenedIOAdapter( const QString& filename, IOAdapterMode mode, TaskStateInfo& tsi ) {
    IOAdapterId adId = IOAdapterUtils::url2io( filename );
    IOAdapterFactory* ioFactory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( adId );
    if( NULL == ioFactory ) {
        tsi.setError(( "cannot_create_io_factory_for_tmp_file" ));
        return NULL;
    }
    
    IOAdapter* io = ioFactory->createIOAdapter();
    if( NULL == io ) {
        tsi.setError(( "cannot_create_io_adapter_for_tmp_file" ));
        return NULL;
    }
    if( !io->open( filename, mode ) ) {
        tsi.setError(QString( "cannot_open_'%1'_file" ).arg( filename ));
        delete io;
        return NULL;
    }
    return io;
}

qint64 fromFileToFile( IOAdapter* from, IOAdapter* to, bool& ok ) {
    assert( NULL != from && NULL != to );
    
    qint64 howMuch = 0;
    int ret = BUF_SZ;
    QByteArray buf( BUF_SZ, 0 );
    while ( BUF_SZ == ret ) {
        ret = from->readBlock( buf.data(), BUF_SZ );
        if( READ_ERR == ret ) {
            ok = false;
            return -1;
        }
        int wrote = to->writeBlock( buf.data(), ret );
        if( ret != wrote ) {
            ok = false;
            return -1;
        }
        howMuch += wrote;
    }
    ok = true;
    return howMuch;
}

qint64 getTotalFilesSize( const QStringList& urls, const QList< int >& numbers ) {
    assert( urls.size() == numbers.size() );
    
    qint64 ret = 0;
    int i = 0;
    int sz = urls.size();
    for( i = 0; i < sz; ++i ) {
        QFile curFile( urls[i] );
        ret += numbers[i] * curFile.size();
    }
    return ret;
}

bool yesNoTag( const QDomElement& el, const QString& tag ) {
    QString val = el.attribute( tag ).toLower();
    return !val.isEmpty() && NO_ANSWER != val && N_ANSWER != val;
}

} // anonymous namespace

namespace U2 {

/************************************************* 
*************create file index test***************
**************************************************
*/
const QString GTest_CreateFileIndex::INPUT_URLS_TAG        = "input";
const QString GTest_CreateFileIndex::TMP_INPUT_TAG         = "tmpInput";
const QString GTest_CreateFileIndex::INPUT_URLS_SEPARATOR  = ",";
const QString GTest_CreateFileIndex::OUTPUT_URL_TAG        = "output";
const QString GTest_CreateFileIndex::DELETE_OUTPUT_TAG     = "delete";

void GTest_CreateFileIndex::init( XMLTestFormat* tf, const QDomElement& el ) {
    Q_UNUSED( tf );
    
    //delOutput = !el.attribute( DELETE_OUTPUT_TAG ).isEmpty();
    delOutput = yesNoTag( el, DELETE_OUTPUT_TAG );
    //tmpInput = !el.attribute( TMP_INPUT_TAG ).isEmpty();
    tmpInput = yesNoTag( el, TMP_INPUT_TAG );
    outputUrl = env->getVar( ENV_TMP_DATA_DIR ) + "/" + el.attribute( OUTPUT_URL_TAG );
    outFactory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( IOAdapterUtils::url2io( outputUrl ) );
    inputUrls = el.attribute( INPUT_URLS_TAG ).split( INPUT_URLS_SEPARATOR, QString::SkipEmptyParts );
}

void GTest_CreateFileIndex::prepare() {
    if( stateInfo.hasError() ) {
        return;
    }
    if( !tmpInput ) {
        addStr( inputUrls, env->getVar( ENV_COMMON_DATA_DIR ) + "/" );
    }
    else {
        QStringList ctxNames = inputUrls;
        inputUrls.clear();
        foreach( const QString& name, ctxNames ) {
            QTemporaryFile* tmpFile = getContext<QTemporaryFile>( this, name );
            if( NULL == tmpFile ) {
                stateInfo.setError(QString( "tmp_file_'%1'_not_found" ).arg( name ));
                return;
            }
            inputUrls.append( tmpFile->fileName() );
        }
    }
    inFactories = getIOAdapterFactories( inputUrls );
    createIndTask = new CreateFileIndexTask( inputUrls, outputUrl, inFactories, outFactory );
    addSubTask( createIndTask );
}

Task::ReportResult GTest_CreateFileIndex::report() {
    stateInfo.setError(createIndTask->getError());
    return ReportResult_Finished;
}

GTest_CreateFileIndex::~GTest_CreateFileIndex() {
    if( delOutput ) {
        QFile::remove( outputUrl );
    }
}

/*
************************************************
*************generate file test*******************
**************************************************
*/

const QString GTest_GenerateFileTest::INPUT_URLS_TAG       = "input";
const QString GTest_GenerateFileTest::INPUT_NUMBERS_TAG    = "amount";
const QString GTest_GenerateFileTest::INPUT_SEPARATOR      = ",";

const QString GTest_GenerateFileTest::OUTPUT_CTX_NAME_TAG     = "outCtxName";
const QString GTest_GenerateFileTest::GZIP_OUTPUT_TAG         = "gzipOut";

void GTest_GenerateFileTest::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED( tf );
    
    tpm = Progress_Manual;
    ctxAdded = false;
    outFile = NULL;
    
    inputUrls = el.attribute( INPUT_URLS_TAG ).split( INPUT_SEPARATOR, QString::SkipEmptyParts );
    if( inputUrls.isEmpty() ) {
        stateInfo.setError(QString( "input_urls_not_specified" ));
        return;
    }
    addStr( inputUrls, env->getVar( ENV_COMMON_DATA_DIR ) + "/" );
    bool numsOk = false;
    QStringList numsList = el.attribute( INPUT_NUMBERS_TAG ).split( INPUT_SEPARATOR, QString::SkipEmptyParts );
    numbers = getNumbers( numsList, numsOk );
    if( !numsOk ) {
        stateInfo.setError( QString( "cannot_parse_numbers_string" ));
        return;
    }
    if( numbers.size() != inputUrls.size() ) {
        stateInfo.setError(QString( "urls_and_numbers_size_do_not_match" ));
        return;
    }
    
    outCtxName = el.attribute( OUTPUT_CTX_NAME_TAG );
    if( outCtxName.isEmpty() ) {
        stateInfo.setError(QString( "context_name_do_not_specified" ));
        return;
    }
    
    outFilenameTemplate = env->getVar( ENV_TMP_DATA_DIR ) + "/" + UGENE_TMP_FILE_TEMPLATE;
    //bool gzipOut = !el.attribute( GZIP_OUTPUT_TAG ).isEmpty();
    bool gzipOut = yesNoTag( el, GZIP_OUTPUT_TAG );
    if( gzipOut ) {
        outFilenameTemplate += GZIP_END;
    }
}

void GTest_GenerateFileTest::prepare() {
    run();
}

void GTest_GenerateFileTest::run() {
    if( stateInfo.hasError() ) {
        return;
    }
    assert( !inputUrls.isEmpty() && !numbers.isEmpty() );
    assert( inputUrls.size() == numbers.size() );
    bool success = false;
    
    outFile = new QTemporaryFile( outFilenameTemplate );
    success = outFile->open();
    if( !success ) {
        stateInfo.setError(QString( "cannot_create_temporary_file" ));
        return;
    }
    outFile->setAutoRemove( true );
    IOAdapter* tmpFileIO = getOpenedIOAdapter( outFile->fileName(), IOAdapterMode_Write, stateInfo );
    if( stateInfo.hasError() ) {
        assert( NULL == tmpFileIO );
        return;
    }
    
    const qint64 totalSz = getTotalFilesSize( inputUrls, numbers );
    qint64 howMuchWritten = 0;
    int i = 0;
    int sz = inputUrls.size();
    for( i = 0; i < sz; ++i ) {
        int j = 0;
        int curNum = numbers[i];
        QString curUrl = inputUrls[i];
        for( j = 0; j < curNum; ++j ) {
            IOAdapter* curIO = getOpenedIOAdapter( curUrl, IOAdapterMode_Read, stateInfo );
            if( stateInfo.hasError() ) {
                assert( NULL == curIO );
                delete tmpFileIO;
                return;
            }
            howMuchWritten += fromFileToFile( curIO, tmpFileIO, success );
            if( !success ) {
                stateInfo.setError(QString( "io_error_occured" ));
                delete tmpFileIO;
                return;
            }
            stateInfo.progress = ( howMuchWritten * 100 ) / totalSz;
            delete curIO;
        }
    }
    delete tmpFileIO;
}

Task::ReportResult GTest_GenerateFileTest::report(){
    if( !stateInfo.hasError() ) {
        addContext( outCtxName, outFile );
        ctxAdded = true;
    }
    return ReportResult_Finished;
}

void GTest_GenerateFileTest::cleanup() {
    if( ctxAdded ) {
        removeContext( outCtxName );
        ctxAdded = false;
    }
    delete outFile;
    outFile = 0;
}

GTest_GenerateFileTest::~GTest_GenerateFileTest() {
    cleanup();
}

/*
********************************************************
*************get document from index test***************
********************************************************
*/

const QString GTest_GetDocFromIndexTest::INDEX_DOCUMENT_TAG = "indDocName";
const QString GTest_GetDocFromIndexTest::DOC_NUMBER_TAG     = "docNum";
const QString GTest_GetDocFromIndexTest::DOC_CTX_NAME_TAG   = "docCtxName";

void GTest_GetDocFromIndexTest::init(XMLTestFormat *tf, const QDomElement& el ) {
    Q_UNUSED( tf );
    
    getDocTask = NULL;
    ctxAdded = false;
    
    indDocName = el.attribute( INDEX_DOCUMENT_TAG );
    if( indDocName.isEmpty() ) {
        stateInfo.setError(QString( "index_context_name_is_empty" ));
        return;
    }
    
    bool ok = false;
    docNum = el.attribute( DOC_NUMBER_TAG ).toInt( &ok );
    if( !ok || 0 > docNum ) {
        stateInfo.setError(QString( "invalid_document_number" ));
        return;
    }
    
    docCtxName = el.attribute( DOC_CTX_NAME_TAG );
    if( docCtxName.isEmpty() ) {
        stateInfo.setError(QString( "document_context_name_is_empty" ));
        return;
    }
}

void GTest_GetDocFromIndexTest::prepare() {
    if( stateInfo.hasError() ) {
        return;
    }
    Document* indDoc = getContext< Document >( this, indDocName );
    if( NULL == indDoc ) {
        stateInfo.setError(QString( "cannot_find_'%1'_in_context" ).arg( indDocName ));
        return;
    }
    QList< GObject* > docObjs = indDoc->getObjects();
    if( 1 != docObjs.size() ) {
        stateInfo.setError(QString( "1_index_object_expected_in_document" ));
        return;
    }
    UIndexObject* indObj = qobject_cast< UIndexObject* >( docObjs[0] );
    if( NULL == indObj ) {
        stateInfo.setError(QString( "cannot_cast_to_index_object" ));
        return;
    }
    
    getDocTask = new GetDocumentFromIndexTask( indObj->getIndex(), docNum );
    addSubTask( getDocTask );
}

Task::ReportResult GTest_GetDocFromIndexTest::report() {
    if ( NULL == getDocTask ) {
        stateInfo.setError(QString( "creating_get_document_from_index_task_failed" ));
        return ReportResult_Finished;
    }
    if( getDocTask->hasError() ) {
        stateInfo.setError(getDocTask->getError());
    }
    else {
        addContext( docCtxName, getDocTask->getDocument() );
        ctxAdded = true;
    }
    return ReportResult_Finished;
}

void GTest_GetDocFromIndexTest::cleanup() {
    if( ctxAdded ) {
        removeContext( docCtxName );
        ctxAdded = false;
    }
}

GTest_GetDocFromIndexTest::~GTest_GetDocFromIndexTest() {
    cleanup();
}

/*
************************************************
*************files indexing tests***************
************************************************
*/

QList< XMLTestFactory* > FilesIndexingTests::createTestFactories() {
    QList< XMLTestFactory* > res;
    res.append( GTest_CreateFileIndex::createFactory() );
    res.append( GTest_GenerateFileTest::createFactory() );
    res.append( GTest_GetDocFromIndexTest::createFactory() );
    return res;
}

} // U2
