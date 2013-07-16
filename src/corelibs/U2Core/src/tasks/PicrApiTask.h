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

#ifndef _U2_PICR_API_TASK_H_
#define _U2_PICR_API_TASK_H_

#include <U2Core/global.h>
#include <U2Core/Task.h>

#include <QtCore/QEventLoop>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

namespace U2 {

class Task;

class U2CORE_EXPORT PicrElement {
public:
    PicrElement(const QString& _accessionNumber = QString(),
                const int _accessionVersion = -1,
                const QString& _databaseName = QString(),
                const QString& _databaseDescription = QString(),
                const int _ncbiId = -1,
                const int _taxonId = -1,
                int _identity = -1);

    bool operator == (const PicrElement& other) const;

    QString accessionNumber;
    int accessionVersion;
    QString databaseName;
    QString databaseDescription;
    int ncbiId;
    int taxonId;
    int identity;
};

class XmlPicrParser {
public:
    enum ResponseType {
        Accession,
        ExactSequence,
        Blast
    };

    XmlPicrParser(ResponseType _responseType);
    void parse(const QByteArray& data);

    QString getError() const;
    void setError(const QString& val);

    QList<PicrElement> getResults();
    bool isResultsEmpty();

private:
    ResponseType responseType;
    QString error;
    QList<PicrElement> results;

    const static QString PICR_ACCESSION_RETURN;
    const static QString PICR_SEQUENCE_RETURN;
    const static QString PICR_BLAST_RETURN;
    const static QString PICR_SEQUENCE;
    const static QString PICR_IDENTICAL_CROSS_REFERENCE;
    const static QString PICR_ACCESSION_NUMBER;
    const static QString PICR_ACCESSION_VERSION;
    const static QString PICR_DATABASE_DESCRIPTION;
    const static QString PICR_DATABASE_NAME;
    const static QString PICR_NCBI_GI;
    const static QString PICR_TAXON_ID;
    const static QString SERVER_ERROR_STRING;
};

class U2CORE_EXPORT ConvertDasIdTask : public Task {
    Q_OBJECT
public:
    ConvertDasIdTask(const QString& resId);
    virtual ~ConvertDasIdTask();

    virtual void run();
    QString getAccessionNumber();

public slots:
    void sl_replyFinished(QNetworkReply* reply);
    void sl_onError(QNetworkReply::NetworkError error);
    void sl_uploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:
    QString getRequestUrlString();

    QString                 resourceId;

    QEventLoop*             loop;
    QNetworkReply*          downloadReply;
    QNetworkAccessManager*  networkManager;

    QString                 accNumber;  //result

    const static QString    baseUrl;
    const static QString    databasePart;
    const static QString    parametersPart;
};

class U2CORE_EXPORT GetDasIdsBySequenceTask : public Task {
    Q_OBJECT
public:
    GetDasIdsBySequenceTask(const QByteArray& _sequence);
    virtual ~GetDasIdsBySequenceTask();

    virtual void run();
    QList<PicrElement> getResults();

public slots:
    void sl_replyFinished(QNetworkReply* reply);
    void sl_onError(QNetworkReply::NetworkError error);
    void sl_uploadProgress(qint64 bytesSent, qint64 bytesTotal);

protected:
    QByteArray              sequence;

private:
    virtual QString getRequestUrlString() = 0;
    virtual XmlPicrParser getParser() = 0;

    QEventLoop*             loop;
    QNetworkReply*          downloadReply;
    QNetworkAccessManager*  networkManager;

    QList<PicrElement>      results;  //result
};

class U2CORE_EXPORT GetDasIdsByExactSequenceTask : public GetDasIdsBySequenceTask {
    Q_OBJECT
public:
    GetDasIdsByExactSequenceTask(const QByteArray& _sequence);
    virtual ~GetDasIdsByExactSequenceTask();

private:
    virtual QString getRequestUrlString();
    virtual XmlPicrParser getParser();

    const static QString    baseUrl;
    const static QString    databasePart;
    const static QString    parametersPart;
};

class U2CORE_EXPORT PicrBlastSettings {
public:
    void insert(const QString& key, const QVariant& value);
    bool isValid();
    QString value(const QString& key);

    const static QString PROGRAM;
    const static QString IDENTITY;
    const static QString MATRIX;
    const static QString FILTER;
    const static QString GAP_OPEN;
    const static QString GAP_EXT;
    const static QString DROP_OFF;
    const static QString GAP_ALIGN;

private:
    QVariantMap settings;
};

class U2CORE_EXPORT GetDasIdsByBlastTask : public GetDasIdsBySequenceTask {
    Q_OBJECT
public:
    GetDasIdsByBlastTask(const QByteArray& _sequence, const PicrBlastSettings& _settings);
    virtual ~GetDasIdsByBlastTask();

private:
    virtual QString getRequestUrlString();
    virtual XmlPicrParser getParser();

    PicrBlastSettings settings;

    const static QString    baseUrl;
    const static QString    databasePart;
    const static QString    parametersPart;
};

} //namespace

#endif // _U2_PICR_API_TASK_H_
