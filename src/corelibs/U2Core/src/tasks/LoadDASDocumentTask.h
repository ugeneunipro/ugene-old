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
#include <U2Core/AnnotationData.h>
#include <U2Core/SaveDocumentTask.h>

#include <QtCore/QEventLoop>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

namespace U2 {

class ConvertDasIdTask;

class XMLDASSequenceParser{
public:
                    XMLDASSequenceParser( );
    void            parse( const QByteArray &data );

    QString         getError( ) const { return error; }
    void            setError( const QString &val ) { error = val; }

    DNASequence *   getSequence( ) { return seq; }
private:
    QString         error;
    DNASequence*    seq;
};

class XMLDASFeaturesParser{
public:
    XMLDASFeaturesParser( );
    void parse( const QByteArray &data );

    QString getError() const { return error; }
    void setError( const QString &val ) { error = val; }

    QMap<QString, QList<AnnotationData> > getAnnotationData( ) { return annotationData; }
private:
    QString error;

    QMap<QString, QList<AnnotationData> > annotationData;

};

class U2CORE_EXPORT LoadDasObjectTask : public Task{
    Q_OBJECT
    Q_DISABLE_COPY(LoadDasObjectTask)
public:
    LoadDasObjectTask (const QString& accId, const DASSource& source, DASObjectType objType);
    virtual ~LoadDasObjectTask();

    virtual void run();

    DNASequence*                                    getSequence();
    const QString&                                  getAccession() const;
    const DASSource&                                getSource() const;
    const QMap<QString, QList<AnnotationData> >&    getAnnotationData( ) const;

public slots:
    void sl_replyFinished(QNetworkReply* reply);
    void sl_onError(QNetworkReply::NetworkError error);
    void sl_uploadProgress( qint64 bytesSent, qint64 bytesTotal);

    void sl_cancelCheck();
    void sl_timeout();

private:
    QString                 accNumber;
    DASSource               source;
    DASObjectType           objectType;

    QEventLoop*             loop;
    QNetworkReply*          downloadReply;
    QNetworkAccessManager*  networkManager;
    QTimer*                 timeoutTimer;
    QTimer*                 cancelCheckTimer;

    DNASequence*            seq;

    QMap<QString, QList<AnnotationData> > annotationData;
};

class U2CORE_EXPORT LoadDasDocumentTask : public BaseLoadRemoteDocumentTask{
    Q_OBJECT
public:
    LoadDasDocumentTask(const QString& accId, const QString& fullPath, const DASSource& referenceSource, const QList<DASSource>& featureSources);

protected:
    virtual void prepare();

    virtual QString getFileFormat(const QString & dbid);
    virtual GUrl    getSourceUrl();
    virtual QString getFileName();

    QList<Task*> onSubTaskFinished(Task* subTask);

    bool isAllDataLoaded();

private:
    void mergeFeatures(const QMap<QString, QList<AnnotationData> >& newAnnotations);

private:
    QString                 accNumber;
    QList<DASSource>        featureSources;
    DASSource               referenceSource;

    LoadDasObjectTask*             loadSequenceTask;
    QList<LoadDasObjectTask*>      loadFeaturesTasks;

    SaveDocumentTask*               saveDocumentTask;

    DNASequence*                   seq;
    QMap<QString, QList<AnnotationData> > annotationData;
}; 

class U2CORE_EXPORT ConvertIdAndLoadDasDocumentTask : public Task {
    Q_OBJECT
public:
    ConvertIdAndLoadDasDocumentTask(const QString& accId, const QString& fullPath, const DASSource& referenceSource, const QList<DASSource>& featureSources, bool convertId = false);
    Document* getDocument() { return loadDasDocumentTask->getDocument(); }
    Document* takeDocument() { return loadDasDocumentTask->takeDocument(); }
    QString getLocalUrl() { return loadDasDocumentTask->getLocalUrl(); }

    virtual void run() {}
    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task *subTask);

private:
    ConvertDasIdTask*       convertDasIdTask;
    LoadDasDocumentTask*    loadDasDocumentTask;

    QString                 accessionNumber;
    QString                 fullPath;
    DASSource               referenceSource;
    QList<DASSource>        featureSources;

    bool convertId;
};


} //namespace

#endif // _U2_LOAD_DAS_DOCUMENT_TASK_H_
