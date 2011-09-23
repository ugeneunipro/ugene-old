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

#ifndef _U2_LOAD_REMOTE_DOCUMENT_TASK_H_
#define _U2_LOAD_REMOTE_DOCUMENT_TASK_H_


#include <U2Core/DocumentProviderTask.h>
#include <U2Core/GUrl.h>

#include <QtCore/QEventLoop>
#include <QtCore/QUrl>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QtXml/QXmlDefaultHandler>
#include <QtXml/QXmlSimpleReader>

namespace U2 {

class Document;
class CopyDataTask;
class LoadDocumentTask;
class LoadDataFromEntrezTask;

class U2CORE_EXPORT RemoteDBRegistry {
    RemoteDBRegistry(); 
    QMap<QString,QString> queryDBs;
    QMap<QString,QString> httpDBs;
    QMap<QString,QString> hints;
    QMap<QString,QString> aliases;
public:
    QString getURL(const QString& accId, const QString& dbName);
    QString getDbEntrezName(const QString& dbName);
    void convertAlias(QString& dbName);
    QList<QString> getDBs();
    QString getHint(const QString& dbName);
    //TODO: move this to AppContext
    static RemoteDBRegistry& getRemoteDBRegistry();
};


class U2CORE_EXPORT RecentlyDownloadedCache : public QObject {
    Q_OBJECT
    QMap<QString, QString> urlMap;
    void loadCacheFromSettings();
    void saveCacheToSettings();
public:
    RecentlyDownloadedCache();
    ~RecentlyDownloadedCache();
    bool contains(const QString& fileName);
    void append(const QString& fullPath);
    void remove(const QString& fullPath);
    QString getFullPath(const QString& fileName);
    void clear() { urlMap.clear(); }

};


class U2CORE_EXPORT LoadRemoteDocumentTask : public DocumentProviderTask {
    Q_OBJECT
public:
    static QString getFileFormat(const QString & dbName);

    bool prepareDownloadDirectory(QString &path);
    static QString getDefaultDownloadDirectory();
    
public:
    LoadRemoteDocumentTask(const GUrl& url);
    LoadRemoteDocumentTask(const QString & accId, const QString & dbName, const QString & fullPathDir = QString());
    virtual void prepare();
    virtual ReportResult report();
    QString getLocalUrl(){ return fullPath; }
    
    QString getAccNumber() const { return accNumber; }
    QString getDBName() const { return dbName; }

protected:
    QList<Task*> onSubTaskFinished(Task* subTask);

private:
    bool openView;
    bool initLoadDocumentTask(); 
    DocumentFormatId formatId;
    CopyDataTask* copyDataTask;
    LoadDocumentTask* loadDocumentTask;
    LoadDataFromEntrezTask* loadDataFromEntrezTask;
    GUrl    sourceUrl;
    QString accNumber, dbName, format;
    QString fileName, fullPath;
};

// This task makes queries to NCBI Entrez search engine, using eTools
// First step: query eSearch to get global Entrez index
// Second step: query eFetch to download file by index
// About eTools: http://www.ncbi.nlm.nih.gov/bookshelf/br.fcgi?book=coursework&part=eutils

class U2CORE_EXPORT LoadDataFromEntrezTask : public Task {
    Q_OBJECT
public:
    LoadDataFromEntrezTask(const QString& dbId, const QString& accNumber, const QString& retType, const QString& fullPath);
    ~LoadDataFromEntrezTask();
    
    virtual void run();
    
public slots:
    void sl_replyFinished(QNetworkReply* reply);
    void sl_onError(QNetworkReply::NetworkError error);
    void sl_uploadProgress( qint64 bytesSent, qint64 bytesTotal);
private:
    QEventLoop* loop;
    QNetworkReply* searchReply;
    QNetworkReply* downloadReply;
    QNetworkAccessManager* networkManager;
    QXmlSimpleReader xmlReader;
    QString db, accNumber;
    bool copyDataMode;
    QString resultIndex;
    QString fullPath, format;
};


// Helper class to parse NCBI Entrez eSearch results
class ESearchResultHandler : public QXmlDefaultHandler {
    bool metESearchResult;
    QString errorStr;
    QString curText;
    QString index;
public:
    ESearchResultHandler();
    bool startElement(const QString &namespaceURI, const QString &localName,
        const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
        const QString &qName);
    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    QString errorString() const { return errorStr; }
    QString getResultIndex() const { return index; }
};

} //namespace

#endif // _U2_LOAD_REMOTE_DOCUMENT_TASK_H_
