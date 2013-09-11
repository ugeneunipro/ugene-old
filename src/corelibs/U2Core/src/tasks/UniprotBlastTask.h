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

#ifndef _U2_UNIPROT_API_TASK_H_
#define _U2_UNIPROT_API_TASK_H_

#include <U2Core/global.h>
#include <U2Core/Task.h>

#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include <QtNetwork/QNetworkReply>

namespace U2 {

class ReplyHandler : public QObject {
    Q_OBJECT
public:
    enum Step {
        Ordered = 1,
        Waiting = 2,
        WaitingComplete = 3,
        DownloadingComplete = 4
    };

    ReplyHandler(const QString &_url, TaskStateInfo* _os);

    QByteArray getReplyData() { return replyData; }
    void sendRequest();
    void setUrl(const QString& _url) { url = _url; }

signals:
    void si_finish();
    void si_step(int step);

private slots:
    void sl_replyFinished(QNetworkReply* reply);
    void sl_onError(QNetworkReply::NetworkError error);
    void sl_uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void sl_timerShouts();

private:
    QNetworkAccessManager* networkManager;
    QString                url;                // url for the next request
    QByteArray             replyData;
    TaskStateInfo*         os;


    static const QByteArray LOCATION;
    static const QByteArray RETRY;
    static const QByteArray CONTENT_TYPE;
    static const QByteArray XML_CONTENT_TYPE;
    static const QByteArray COMPLETED;
    static const QByteArray RUNNING;
};

class U2CORE_EXPORT UniprotResult {
public:
    UniprotResult() :
        hitNumber(-1),
        length(-1),
        alignmentsTotal(-1),
        alignmentsNumber(-1),
        score(-1),
        bits(-1),
        expectation(-1),
        identity(-1),
        positives(-1),
        gaps(-1),
        querySeqStart(-1),
        querySeqEnd(-1),
        matchSeqStart(-1),
        matchSeqEnd(-1) {}

    int hitNumber;
    QString database;
    QString id;
    QString accession;
    int length;
    QString description;
    int alignmentsTotal;
    int alignmentsNumber;
    int score;
    double bits;
    double expectation;
    double identity;
    double positives;
    int gaps;
    QString strand;

    int querySeqStart;
    int querySeqEnd;
    QString querySeq;

    QString pattern;

    int matchSeqStart;
    int matchSeqEnd;
    QString matchSeq;
};

class XmlUniprotParser {
public:
    void parse(const QByteArray& data);

    const QString& getError() const { return error; }
    void setError(const QString& val) { error = val; }
    bool hasError() const { return !error.isEmpty(); }

    QList<UniprotResult> getResults() const { return results; }
    bool isResultsEmpty() const { return results.isEmpty(); }

private:
    QString error;
    QList<UniprotResult> results;

    static const QString EBI_APPLICATION_RESULT;
    static const QString SEQUENCE_SIMILARITY_SEARCH_RESULT;
    static const QString HITS;
    static const QString HITS_TOTAL;
    static const QString HIT;
    static const QString HIT_NUMBER;
    static const QString HIT_DATABASE;
    static const QString HIT_ID;
    static const QString HIT_AC;
    static const QString HIT_LENGTH;
    static const QString HIT_DESCRIPTION;
    static const QString ALIGNMENTS;
    static const QString ALIGNMENTS_TOTAL;
    static const QString ALIGNMENT;
    static const QString ALIGNMENT_NUMBER;
    static const QString SCORE;
    static const QString BITS;
    static const QString EXPECTATION;
    static const QString IDENTITY;
    static const QString POSITIVES;
    static const QString GAPS;
    static const QString STRAND;
    static const QString QUERY_SEQ;
    static const QString QUERY_SEQ_START;
    static const QString QUERY_SEQ_END;
    static const QString PATTERN;
    static const QString MATCH_SEQ;
    static const QString MATCH_SEQ_START;
    static const QString MATCH_SEQ_END;
};

class U2CORE_EXPORT UniprotBlastSettings {
public:
    void insert(const QString& key, const QString& value) { settings.insert(key, value); }
    bool isValid() const;
    QString getString() const;       // unsafe, call isValid() manually

    const static QString DATABASE;
    const static QString THRESHOLD;
    const static QString MATRIX;
    const static QString FILTERING;
    const static QString GAPPED;
    const static QString HITS;

    const static QString DEFAULT_DATABASE;
    const static QString DEFAULT_THRESHOLD;
    const static QString DEFAULT_MATRIX;
    const static QString DEFAULT_FILTERING;
    const static QString DEFAULT_GAPPED;
    const static QString DEFAULT_HITS;

    const static QStringList ALLOWED_DATABASE;
    const static QStringList ALLOWED_THRESHOLD;
    const static QStringList ALLOWED_MATRIX;
    const static QStringList ALLOWED_FILTERING;
    const static QStringList ALLOWED_GAPPED;
    const static QStringList ALLOWED_HITS;

private:
    QStrStrMap settings;
};

class U2CORE_EXPORT UniprotBlastTask : public Task {
    Q_OBJECT
public:
    UniprotBlastTask(const QByteArray& _sequence, const UniprotBlastSettings& _settings);
    ~UniprotBlastTask();

    virtual void run();
    QList<UniprotResult> getResults() const { return results; }

public slots:
    void sl_exitLoop();
    void sl_stateChanged(int step);

private:
    QString generateUrl();

    QByteArray              sequence;
    UniprotBlastSettings    settings;

    QEventLoop*             loop;               // main task loop: enter at the task beginning, left on task finish or cancelling
    QList<UniprotResult>    results;

    static const QByteArray BASE_URL;
};

}   // namespace U2

#endif // _U2_UNIPROT_API_TASK_H_
