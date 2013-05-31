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

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/NetworkConfiguration.h>
#include <U2Core/Counter.h>
#include <U2Core/DBXRefRegistry.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/CopyDataTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>


#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QUrl>


#include "LoadRemoteDocumentTask.h"

namespace U2 {

const QString NCBI_ESEARCH_URL("http://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi?db=%1&term=%2&tool=UGENE");
const QString NCBI_EFETCH_URL("http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=%1&id=%2&retmode=text&rettype=%3&tool=UGENE");
const QString RemoteDBRegistry::ENSEMBL("ENSEMBL");
const QString RemoteDBRegistry::GENBANK_DNA("NCBI GenBank (DNA sequence)");
const QString RemoteDBRegistry::GENBANK_PROTEIN("NCBI protein sequence database");
const QString RemoteDBRegistry::PDB("PDB");
const QString RemoteDBRegistry::SWISS_PROT("SWISS-PROT");
const QString RemoteDBRegistry::UNIPROTKB_SWISS_PROT("UniProtKB/Swiss-Prot");
const QString RemoteDBRegistry::UNIPROTKB_TREMBL("UniProtKB/TrEMBL");


////////////////////////////////////////////////////////////////////////////
//BaseLoadRemoteDocumentTask
BaseLoadRemoteDocumentTask::BaseLoadRemoteDocumentTask(const QString& _downloadPath)
:DocumentProviderTask(tr("Load remote document"), TaskFlags_NR_FOSCOE | TaskFlag_MinimizeSubtaskErrorText)
{
    downloadPath = _downloadPath;
    sourceUrl = GUrl("");
    fullPath = "";
    fileName = "";
}

void BaseLoadRemoteDocumentTask::prepare(){
    sourceUrl = getSourceURL();
    fileName = getFileName();

    if (!downloadPath.isEmpty()) {
        fullPath = QDir::cleanPath(downloadPath);
        fullPath = !fullPath.endsWith("/") ? fullPath + "/" : fullPath;
    }

    if (fileName.isEmpty()) {
        stateInfo.setError("Incorrect key identifier!");
        return;
    }

    if (fullPath.isEmpty()) {
        fullPath = getDefaultDownloadDirectory();
    }

    if (!prepareDownloadDirectory(fullPath)) {
        setError(QString("Directory %1 does not exist").arg(fullPath));
        return;
    }

    fullPath += "/" + fileName;

}

Task::ReportResult BaseLoadRemoteDocumentTask::report()
{
    return ReportResult_Finished;
}

bool BaseLoadRemoteDocumentTask::prepareDownloadDirectory( QString &path ){
    if (!QDir(path).exists()) {
        if (path == getDefaultDownloadDirectory()) {
            // Creating default directory if it doesn't exist
            if (!QDir().mkpath(path)) {
                return false;
            }
        }
        else {
            // We do not touch user specified directories from here
            return false;
        }
    }

    return true;
}

QString BaseLoadRemoteDocumentTask::getDefaultDownloadDirectory(){
    QString path = AppContext::getAppSettings()->getUserAppsSettings()->getDownloadDirPath();
    return path;
}

bool BaseLoadRemoteDocumentTask::initLoadDocumentTask(){
    // Check if the document has been loaded 
    Project* proj = AppContext::getProject();
    if (proj != NULL) {
        resultDocument = proj->findDocumentByURL(fullPath);
        if (resultDocument != NULL) {
            docOwner = false;
            return false;
        }
    }

    // Detect format
    if (formatId.isEmpty()) {
        QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(fullPath);
        CHECK_EXT(!formats.isEmpty(), setError(tr("Unknown file format!")), false)
            formatId = formats.first().format->getFormatId();
    }
    IOAdapterFactory * iow = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    loadDocumentTask = new LoadDocumentTask(formatId, fullPath, iow);

    return true;
}

bool BaseLoadRemoteDocumentTask::isCached(){
    // Check if the file has already been downloaded
    RecentlyDownloadedCache* cache = AppContext::getRecentlyDownloadedCache();
    if( cache != NULL && cache->contains(fileName)) {
        QString cachedUrl = cache->getFullPath(fileName);
        if( fullPath == cachedUrl ) {
            if (initLoadDocumentTask() ) {
                addSubTask(loadDocumentTask);
            } 
            return true;
        } // else: user wants to save doc to new file -> download it from db
    }

    return false;
}

void BaseLoadRemoteDocumentTask::createLoadedDocument(){
    GUrl url(fullPath);
    // Detect format
    if (formatId.isEmpty()) {
        formatId = BaseDocumentFormats::PLAIN_GENBANK;
    }
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    DocumentFormat* dformat = AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
    U2OpStatus2Log os;
    resultDocument = dformat->createNewLoadedDocument(iof, url, os);
}

//////////////////////////////////////////////////////////////////////////
//LoadRemoteDocumentTask
LoadRemoteDocumentTask::LoadRemoteDocumentTask( const GUrl& url )
:BaseLoadRemoteDocumentTask()
{
    fileUrl = url;
    GCOUNTER( cvar, tvar, "LoadRemoteDocumentTask" );
}

LoadRemoteDocumentTask::LoadRemoteDocumentTask( const QString & accId, const QString & dbName, const QString & fullPathDir)
:BaseLoadRemoteDocumentTask(fullPathDir)
,accNumber(accId)
,dbName(dbName)
{
    GCOUNTER( cvar, tvar, "LoadRemoteDocumentTask" );

}

void LoadRemoteDocumentTask::prepare(){
    BaseLoadRemoteDocumentTask::prepare();
    if (!isCached()){
        if (sourceUrl.isHyperLink()) {
            IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::HTTP_FILE);
            IOAdapterFactory * iow = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            copyDataTask = new CopyDataTask(iof, sourceUrl, iow, fullPath);
            addSubTask(copyDataTask);
        } else {
            assert(sourceUrl.isLocalFile());
            QString dbId = RemoteDBRegistry::getRemoteDBRegistry().getDbEntrezName(dbName);
            if(dbId.isEmpty()) {
                setError(tr("Undefined database: '%1'").arg(dbName));
                return;
            } else {
                loadDataFromEntrezTask = new LoadDataFromEntrezTask(dbId, accNumber,format,fullPath);
                addSubTask(loadDataFromEntrezTask);
            }
        }
    }
}

QString LoadRemoteDocumentTask::getFileFormat( const QString & dbid ){
    QString dbId = RemoteDBRegistry::getRemoteDBRegistry().getDbEntrezName(dbid);
    if (dbId == GENBANK_NUCLEOTIDE_ID || dbId == GENBANK_PROTEIN_ID) {
        return GENBANK_FORMAT;
    } else {
        return FASTA_FORMAT;
    }
}

GUrl LoadRemoteDocumentTask::getSourceURL(){
    if (!fileUrl.isEmpty()){
        return fileUrl;
    }else{
        RemoteDBRegistry::getRemoteDBRegistry().convertAlias(dbName);
        return GUrl(RemoteDBRegistry::getRemoteDBRegistry().getURL(accNumber, dbName));    
    }
}

QString LoadRemoteDocumentTask::getFileName(){
    
    if( sourceUrl.isHyperLink() ) {
        return sourceUrl.fileName();
    } else {
        format = getFileFormat(dbName);
        accNumber.replace(";",",");
        QStringList accIds = accNumber.split(",");
        if (accIds.size() == 1 ) {
            return accNumber + "." + format;
        } else if (accIds.size() > 1) {
            return accIds.first() + "_misc." + format;
        }
    }

    return "";
}

QList<Task*> LoadRemoteDocumentTask::onSubTaskFinished( Task* subTask ){
    QList<Task*> subTasks;
    if (subTask->hasError()) {
        if( subTask == copyDataTask || subTask == loadDataFromEntrezTask ) {
            setError(tr("Cannot find %1 in %2 database").arg(accNumber).arg(dbName) + ": " + subTask->getError());
        }
        return subTasks;
    }
    if (subTask == copyDataTask || subTask == loadDataFromEntrezTask) {
        if (initLoadDocumentTask()) {
            subTasks.append(loadDocumentTask);
            if (!subTask->isCanceled()) {
                RecentlyDownloadedCache * cache = AppContext::getRecentlyDownloadedCache();
                if(cache != NULL) {
                    cache->append(fullPath);
                }
            } else if (subTask == copyDataTask) {
                QFile notLoadedFile(fullPath);
                notLoadedFile.remove();
            }
        }
    } else if ( subTask == loadDocumentTask) {
        resultDocument = loadDocumentTask->takeDocument();
    }
    return subTasks;
}
/*
LoadRemoteDocumentTask::LoadRemoteDocumentTask( const GUrl& fileUrl) 
: DocumentProviderTask(tr("Load remote document"), TaskFlags_NR_FOSCOE | TaskFlag_MinimizeSubtaskErrorText), copyDataTask(NULL),
loadDocumentTask(NULL) 
{
    
    sourceUrl = fileUrl;
    fileName = sourceUrl.fileName();
    GCOUNTER( cvar, tvar, "LoadRemoteDocumentTask" );
}

LoadRemoteDocumentTask::LoadRemoteDocumentTask(const QString & accId, const QString & dbNm, const QString & fullPathDir)
: DocumentProviderTask(tr("Load remote document"), TaskFlags_NR_FOSCOE | TaskFlag_MinimizeSubtaskErrorText), copyDataTask(NULL), 
loadDocumentTask(NULL), accNumber(accId), dbName(dbNm) 
{
    
    RemoteDBRegistry::getRemoteDBRegistry().convertAlias(dbName);
    sourceUrl = GUrl(RemoteDBRegistry::getRemoteDBRegistry().getURL(accId, dbName));
    if( sourceUrl.isHyperLink() ) {
        fileName = sourceUrl.fileName();
    } else {
        format = getFileFormat(dbName);
        accNumber.replace(";",",");
        QStringList accIds = accNumber.split(",");
        if (accIds.size() == 1 ) {
            fileName = accNumber + "." + format;
        } else if (accIds.size() > 1) {
            fileName = accIds.first() + "_misc." + format;
        }
    }
    
    if (!fullPathDir.isEmpty()) {
        fullPath = QDir::cleanPath(fullPathDir);
        fullPath = !fullPath.endsWith("/") ? fullPath + "/" : fullPath;
    }

    GCOUNTER( cvar, tvar, "LoadRemoteDocumentTask" );
}

QString LoadRemoteDocumentTask::getFileFormat(const QString & dbName) {
    QString dbId = RemoteDBRegistry::getRemoteDBRegistry().getDbEntrezName(dbName);
    if (dbId == GENBANK_NUCLEOTIDE_ID || dbId == GENBANK_PROTEIN_ID) {
        return GENBANK_FORMAT;
    } else {
        return FASTA_FORMAT;
    }
}

bool LoadRemoteDocumentTask::prepareDownloadDirectory(QString &path) {
    if (!QDir(path).exists()) {
        if (path == getDefaultDownloadDirectory()) {
            // Creating default directory if it doesn't exist
            if (!QDir().mkpath(path)) {
                return false;
            }
        }
        else {
            // We do not touch user specified directories from here
            return false;
        }
    }

    return true;
}

QString LoadRemoteDocumentTask::getDefaultDownloadDirectory() {
    QString path = AppContext::getAppSettings()->getUserAppsSettings()->getDownloadDirPath();
    return path;
}

void LoadRemoteDocumentTask::prepare() {
    if (fileName.isEmpty()) {
        stateInfo.setError("Incorrect key identifier!");
        return;
    }
    
    if (fullPath.isEmpty()) {
        fullPath = getDefaultDownloadDirectory();
    }

    if (!prepareDownloadDirectory(fullPath)) {
        setError(QString("Directory %1 does not exist").arg(fullPath));
        return;
    }

    fullPath += "/" + fileName;
    
    // Check if the file has already been downloaded
    RecentlyDownloadedCache* cache = AppContext::getRecentlyDownloadedCache();
    if( cache != NULL && cache->contains(fileName)) {
        QString cachedUrl = cache->getFullPath(fileName);
        if( fullPath == cachedUrl ) {
            if (initLoadDocumentTask() ) {
                addSubTask(loadDocumentTask);
            } 
            return;
        } // else: user wants to save doc to new file -> download it from db
    }
    
    if (sourceUrl.isHyperLink()) {
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::HTTP_FILE);
        IOAdapterFactory * iow = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        copyDataTask = new CopyDataTask(iof, sourceUrl, iow, fullPath);
        addSubTask(copyDataTask);
    } else {
        assert(sourceUrl.isLocalFile());
        QString dbId = RemoteDBRegistry::getRemoteDBRegistry().getDbEntrezName(dbName);
        if(dbId.isEmpty()) {
            setError(tr("Undefined database: '%1'").arg(dbName));
            return;
        } else {
            loadDataFromEntrezTask = new LoadDataFromEntrezTask(dbId, accNumber,format,fullPath);
            addSubTask(loadDataFromEntrezTask);
        }
    }
}

Task::ReportResult LoadRemoteDocumentTask::report()
{
    return ReportResult_Finished;
}

QList<Task*> LoadRemoteDocumentTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> subTasks;
    if (subTask->hasError()) {
        if( subTask == copyDataTask || subTask == loadDataFromEntrezTask ) {
            setError(tr("Cannot find %1 in %2 database").arg(accNumber).arg(dbName) + ": " + subTask->getError());
        }
        return subTasks;
    }
    if (subTask == copyDataTask || subTask == loadDataFromEntrezTask) {
        if (initLoadDocumentTask()) {
            subTasks.append(loadDocumentTask);
            if (!subTask->isCanceled()) {
                RecentlyDownloadedCache * cache = AppContext::getRecentlyDownloadedCache();
                if(cache != NULL) {
                    cache->append(fullPath);
                }
            } else if (subTask == copyDataTask) {
                QFile notLoadedFile(fullPath);
                notLoadedFile.remove();
            }
        }
    } else if ( subTask == loadDocumentTask) {
        resultDocument = loadDocumentTask->takeDocument();
    }
    return subTasks;
}

bool LoadRemoteDocumentTask::initLoadDocumentTask() {
    // Check if the document has been loaded 
    Project* proj = AppContext::getProject();
    if (proj != NULL) {
        resultDocument = proj->findDocumentByURL(fullPath);
        if (resultDocument != NULL) {
            docOwner = false;
            return false;
        }
    }

    // Detect format
    if (formatId.isEmpty()) {
        QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(fullPath);
        CHECK_EXT(!formats.isEmpty(), setError(tr("Unknown file format!")), false)
        formatId = formats.first().format->getFormatId();
    }
    IOAdapterFactory * iow = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    loadDocumentTask = new LoadDocumentTask(formatId, fullPath, iow);

    return true;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
RecentlyDownloadedCache::RecentlyDownloadedCache() {
    QStringList fileNames = AppContext::getAppSettings()->getUserAppsSettings()->getRecentlyDownloadedFileNames();
    foreach (const QString& path, fileNames) {
        QFileInfo info(path);
        if (info.exists()) {
            append(path);
        }
    }
}

bool RecentlyDownloadedCache::contains(const QString& fileName) {
    if (!urlMap.contains(fileName)) {
        return false;
    } else {
        QString fullPath=getFullPath(fileName);
        QFile cachedFile(fullPath);
        return cachedFile.exists();
    }
}

void RecentlyDownloadedCache::append( const QString& fileName ) {
    QFileInfo info(fileName);
    urlMap.insert(info.fileName(), fileName);
}

void RecentlyDownloadedCache::remove(const QString& fullPath) {
    urlMap.remove(QFileInfo(fullPath).fileName());
}

QString RecentlyDownloadedCache::getFullPath( const QString& fileName ) {
    return urlMap.value(fileName);
}

RecentlyDownloadedCache::~RecentlyDownloadedCache() {
    //TODO: cache depends on AppSettings! get rid of this dependency!
    QStringList fileNames = urlMap.values();
    AppSettings* settings = AppContext::getAppSettings();
    UserAppsSettings* us = settings->getUserAppsSettings();
    us->setRecentlyDownloadedFileNames(fileNames);
}

//////////////////////////////////////////////////////////////////////////


LoadDataFromEntrezTask::LoadDataFromEntrezTask( const QString& dbId, const QString& accNum, const QString& retType, const QString& path )
: Task("LoadDataFromEntrez", TaskFlags_FOSCOE | TaskFlag_MinimizeSubtaskErrorText), db(dbId), 
accNumber(accNum), fullPath(path), format(retType) {

}

LoadDataFromEntrezTask::~LoadDataFromEntrezTask() {
    delete loop;
    delete networkManager;
}

void LoadDataFromEntrezTask::run() {
    stateInfo.progress = 0;
    ioLog.trace("Load data from Entrez started...");
    loop = new QEventLoop;

    networkManager = new QNetworkAccessManager();
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(sl_replyFinished(QNetworkReply*)));
    
    NetworkConfiguration* nc = AppContext::getAppSettings()->getNetworkConfiguration();
    

    //TODO: use eSearch functionality for better request handling
    // Step zero: search for global Entrez index
    /*ioLog.trace("Request for global entrez index info...");
    QString traceSearchUrl= QString(NCBI_ESEARCH_URL).arg(db).arg(accNumber);
    ioLog.trace(traceSearchUrl);

    QUrl request( NCBI_ESEARCH_URL.arg(db).arg(accNumber) );
    QString rUrl(request.toString());
    ioLog.trace(QString("Sending request: %1").arg(rUrl));
    searchReply = networkManager->get(QNetworkRequest( request ));
    connect(searchReply, SIGNAL(error(QNetworkReply::NetworkError)),
        this, SLOT(sl_onError(QNetworkReply::NetworkError)));
    loop->exec();
    
    if (resultIndex.isEmpty()) {
        stateInfo.setError("Result not found");
    }
    if (stateInfo.hasError()  ) {
        return;
    }
    
    ioLog.trace(QString("Global index is %1").arg(resultIndex));
    */
    
    ioLog.trace("Downloading file...");
    // Step one: download the file    
    QString traceFetchUrl = QString(NCBI_EFETCH_URL).arg(db).arg(accNumber).arg(format);
    QNetworkProxy proxy = nc->getProxyByUrl(traceFetchUrl);
    networkManager->setProxy(proxy);
    ioLog.trace(traceFetchUrl);
    QUrl requestUrl(NCBI_EFETCH_URL.arg(db).arg(accNumber).arg(format));
    downloadReply = networkManager->get(QNetworkRequest(requestUrl));
    connect(downloadReply, SIGNAL(error(QNetworkReply::NetworkError)),
        this, SLOT(sl_onError(QNetworkReply::NetworkError)));
    connect( downloadReply, SIGNAL(uploadProgress( qint64, qint64 )),
        this, SLOT(sl_uploadProgress(qint64,qint64)) );

    loop->exec();
    ioLog.trace("Download finished.");
    
    QByteArray result = downloadReply->readAll();
    if ( ( result.size() < 100 ) && result.contains("Nothing has been found")) {
        setError(tr("Sequence with ID=%1 is not found.").arg(accNumber));
        return;
    }

    QFile downloadedFile(fullPath);
    if (!downloadedFile.open(QIODevice::WriteOnly)) {
        stateInfo.setError("Cannot open file to write!");
        return;
    }
    
    downloadedFile.write(result);
    downloadedFile.close();
}

void LoadDataFromEntrezTask::sl_replyFinished( QNetworkReply* reply ) {
    if (reply == searchReply) {
        QXmlInputSource source(reply);
        ESearchResultHandler* handler = new ESearchResultHandler;
        xmlReader.setContentHandler(handler);
        xmlReader.setErrorHandler(handler);
        bool ok = xmlReader.parse(source);
        if (!ok) {
            assert(0);
            stateInfo.setError("Parsing eSearch result failed");
        } else {
            resultIndex =  handler->getResultIndex();
        }
        delete handler;

    } else if (reply == downloadReply) {
        
    }
    loop->exit();
}

void LoadDataFromEntrezTask::sl_onError( QNetworkReply::NetworkError error ) {
    stateInfo.setError(QString("NetworkReply error %1").arg(error));
    loop->exit();
}

void LoadDataFromEntrezTask::sl_uploadProgress( qint64 bytesSent, qint64 bytesTotal ) {
    stateInfo.progress = bytesSent/ bytesTotal * 100;
}

//////////////////////////////////////////////////////////////////////////

bool ESearchResultHandler::startElement( const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes )
{
    Q_UNUSED(namespaceURI); Q_UNUSED(localName); Q_UNUSED(attributes);

    if (!metESearchResult && qName != "eSearchResult") {
        errorStr = QObject::tr("This is not ESearch result!");
        return false;
    }
    if ("eSearchResult" == qName) {
        metESearchResult = true;
    } 
    curText.clear();
    return true;
}

bool ESearchResultHandler::endElement( const QString &namespaceURI, const QString &localName, const QString &qName )
{
    Q_UNUSED(namespaceURI); Q_UNUSED(localName);
    if ("Id" == qName) {
        index = curText;
    }
    return true;
}

ESearchResultHandler::ESearchResultHandler()
{

    metESearchResult = false;
}

bool ESearchResultHandler::characters( const QString &str )
{
    curText += str;
    return true;
}

bool ESearchResultHandler::fatalError( const QXmlParseException &exception )
{
    Q_UNUSED(exception);
    assert(0);
    return false;

}

//////////////////////////////////////////////////////////////////////////

RemoteDBRegistry::RemoteDBRegistry() {
    queryDBs.insert(GENBANK_DNA,  GENBANK_NUCLEOTIDE_ID);
    queryDBs.insert(GENBANK_PROTEIN, GENBANK_PROTEIN_ID);

    aliases.insert("ENSEMBL", ENSEMBL);
    aliases.insert("genbank", GENBANK_DNA);
    aliases.insert("genbank-protein", GENBANK_PROTEIN);
    aliases.insert("pdb", PDB);
    aliases.insert("swissprot", SWISS_PROT);
    aliases.insert("uniprot", UNIPROTKB_SWISS_PROT);
    aliases.insert("nucleotide", GENBANK_DNA);
    aliases.insert("protein", GENBANK_PROTEIN);

    const QMap<QString,DBXRefInfo>& entries = AppContext::getDBXRefRegistry()->getEntries();
    foreach(const DBXRefInfo& info, entries.values()) {
        if (!info.fileUrl.isEmpty()) {
            httpDBs.insert(info.name, info.fileUrl);
        }
    }

    hints.insert(ENSEMBL, QObject::tr("Use Ensembl ID. For example: ENSG00000258664 or ENSG00000146463"));
    hints.insert(GENBANK_DNA, QObject::tr("Use Genbank DNA accession number. For example: NC_001363 or D11266"));
    hints.insert(GENBANK_PROTEIN, QObject::tr("Use Genbank protein accession number. For example: AAA59172.1"));
    hints.insert(PDB, QObject::tr("Use PDB molecule four-letter identifier. For example: 3INS or 1CRN"));
    hints.insert(SWISS_PROT, QObject::tr("Use SWISS-PROT accession number. For example: Q9IGQ6 or A0N8V2"));
    hints.insert(UNIPROTKB_SWISS_PROT, QObject::tr("Use UniProtKB/Swiss-Prot accession number. For example: P16152"));
    hints.insert(UNIPROTKB_TREMBL, QObject::tr("Use UniProtKB/TrEMBL accession number. For example: D0VTW9"));
}

RemoteDBRegistry& RemoteDBRegistry::getRemoteDBRegistry() {
    static RemoteDBRegistry registry;
    return registry;
}


QList<QString> RemoteDBRegistry::getDBs() {
    return  ( queryDBs.keys() + httpDBs.keys() );
}

QString RemoteDBRegistry::getURL( const QString& accId, const QString& dbName ) {
    QString result("");
    if (httpDBs.contains(dbName)) {
        result = QString(httpDBs.value(dbName)).arg(accId);
    }    
    return result; 
}

QString RemoteDBRegistry::getDbEntrezName( const QString& dbName ){
    return queryDBs.value(dbName);
}

QString RemoteDBRegistry::getHint( const QString& dbName ) {
    if (hints.contains(dbName)) {
        return hints.value(dbName);
    } else {
        return QObject::tr("Use %1 unique identifier.").arg(dbName);
    }

}

void RemoteDBRegistry::convertAlias( QString& dbName ) {
    if (aliases.contains(dbName)) {
        dbName = aliases.value(dbName);
    }
}

bool RemoteDBRegistry::hasDbId(const QString& dbId){
    return queryDBs.contains(dbId) || httpDBs.contains(dbId);
}

} //namespace
