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

#ifndef _U2_LOAD_DAS_DOCUMENT_TASK_H_
#define _U2_LOAD_DAS_DOCUMENT_TASK_H_

#include <U2Core/global.h>

#include <U2Core/LoadRemoteDocumentTask.h>
#include <U2Core/DASSource.h>
#include <U2Core/DNASequence.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/SaveDocumentTask.h>

#include <QtCore/QEventLoop>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

namespace U2 {

class ConvertDasIdTask;

class XMLDASSequenceParser{
public:
    XMLDASSequenceParser();
    void parse(const QByteArray& data);

    QString getError() const { return error; }
    void setError(const QString& val) { error = val; }

    DNASequence* getSequence(){return seq;}
private:
    QString         error;
    DNASequence*    seq;
    
};

class XMLDASFeaturesParser{
public:
    XMLDASFeaturesParser();
    void parse(const QByteArray& data);

    QString getError() const { return error; }
    void setError(const QString& val) { error = val; }

    QMap<QString, QList<SharedAnnotationData> > getAnnotationData(){return annotationData;}
private:
    QString error;

    QMap<QString, QList<SharedAnnotationData> > annotationData;

};

class U2CORE_EXPORT LoadDASObjectTask : public Task{
    Q_OBJECT
public:
    LoadDASObjectTask (const QString& accId, const DASSource& source, DASObjectType objType);
    virtual ~LoadDASObjectTask();

    virtual void run();

    DNASequence*    getSequence(){return seq;}
    QMap<QString, QList<SharedAnnotationData> >    getAnnotationData(){return annotationData;}
public slots:
    void sl_replyFinished(QNetworkReply* reply);
    void sl_onError(QNetworkReply::NetworkError error);
    void sl_uploadProgress( qint64 bytesSent, qint64 bytesTotal);

private:
    QString                 accNumber;
    DASSource               source;
    DASObjectType           objectType;

    QEventLoop*             loop;
    QNetworkReply*          downloadReply;
    QNetworkAccessManager*  networkManager;

    DNASequence*            seq;

    QMap<QString, QList<SharedAnnotationData> > annotationData;
};

class U2CORE_EXPORT LoadDASDocumentTask : public BaseLoadRemoteDocumentTask{
    Q_OBJECT
public:
    LoadDASDocumentTask(const QString& accId, const QString& fullPath, const DASSource& referenceSource, const QList<DASSource>& featureSources);

protected:
    virtual void prepare();

    virtual QString getFileFormat(const QString & dbid);
    virtual GUrl    getSourceURL();
    virtual QString getFileName();

    QList<Task*> onSubTaskFinished(Task* subTask);

    bool isAllDataLoaded();

private:
    void mergeFeatures(const QMap<QString, QList<SharedAnnotationData> >& newAnnotations);

private:
    QString                 accNumber;
    QList<DASSource>        featureSources;
    DASSource               referenceSource;

    LoadDASObjectTask*             loadSequenceTask;
    QList<LoadDASObjectTask*>      loadFeaturesTasks;

    SaveDocumentTask*               saveDocumentTask;

    DNASequence*                   seq;
    QMap<QString, QList<SharedAnnotationData> > annotationData;
}; 

class U2CORE_EXPORT ConvertIdAndLoadDASDocumentTask : public Task {
    Q_OBJECT
public:
    ConvertIdAndLoadDASDocumentTask(const QString& accId, const QString& fullPath, const DASSource& referenceSource, const QList<DASSource>& featureSources);
    Document* getDocument() { return loadDasDocumentTask->getDocument(); }
    Document* takeDocument() { return loadDasDocumentTask->takeDocument(); }
    QString getLocalUrl() { return loadDasDocumentTask->getLocalUrl(); }

    virtual void run() {}
    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task *subTask);

private:
    ConvertDasIdTask*       convertDasIdTask;
    LoadDASDocumentTask*    loadDasDocumentTask;

    QString                 accessionNumber;
    QString                 fullPath;
    DASSource               referenceSource;
    QList<DASSource>        featureSources;
};


} //namespace

#endif // _U2_LOAD_DAS_DOCUMENT_TASK_H_
