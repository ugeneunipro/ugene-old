/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <U2Core/DASSource.h>
#include <U2Core/AnnotationTableObject.h>

#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QAuthenticator>

namespace U2 {

#define MIN_SEQ_LENGTH 4
#define MAX_SEQ_LENGTH 1900 //because a GET request can handle only 2048 characters

class ReplyHandler : public QObject {
    Q_OBJECT
public:
    enum ReplyState {
        Ordered = 1,
        Waiting = 2,
        WaitingComplete = 3,
        DownloadingComplete = 4
    };

    ReplyHandler(const QString &_url, TaskStateInfo* _os);

    QByteArray getReplyData() { return result; }
    void sendRequest();
    void setUrl(const QString& _url) { url = _url; }

signals:
    void si_finish();
    void si_stateChanged(ReplyHandler::ReplyState state);

private slots:
    void sl_replyFinished();
    void sl_onError(QNetworkReply::NetworkError error);
    void sl_onReadyRead();
    void sl_timerShouts();
    void sl_timeout();
    void onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*);

private:
    void readData(QNetworkReply *reply);
    static void registerMetaType();
    static bool isMetaRegistered;

    QNetworkAccessManager* networkManager;
    QString                url;                // url for the next request
    QByteArray             result;
    QByteArray             replyData;
    TaskStateInfo*         os;
    QTimer                 timer;
    int                    curAttemptNumber;

    static const int        MAX_ATTEMPT_NUMBER;
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
    void sl_stateChanged(ReplyHandler::ReplyState state);

private:
    QString generateUrl();

    QByteArray              sequence;
    UniprotBlastSettings    settings;

    QEventLoop*             loop;               // main task loop: enter at the task beginning, left on task finish or canceling
    QList<UniprotResult>    results;

    static const QByteArray BASE_URL;
};

class U2CORE_EXPORT DASAnnotationsSettings {
public:

    DASAnnotationsSettings()
        :identityThreshold(0)
        ,maxResults(1){}
    QByteArray sequence;
    UniprotBlastSettings blastSettings;
    QList<DASSource> featureSources;
    qint64 identityThreshold;
    qint64 maxResults;
};

typedef QMap<QString, QList<AnnotationData> > DASGroup;

class U2CORE_EXPORT DASAnnotationData {
public:
    DASAnnotationData( qint64 _seqLen, qint64 _identityThreshold )
        : seqLen( _seqLen ), identityThreshold( _identityThreshold ) { }

    QStringList getAccessionNumbers( );
    DASGroup getDasGroup( const QString &accNumber );
    void addDasGroup( const QString &accNumber, const DASGroup &dasGroup );
    bool contains ( const QString &accessionNumber );
    QList<AnnotationData> prepareResults( );

private:
    QMap<QString, DASGroup> dasData;
    qint64 seqLen;
    qint64 identityThreshold;
};

class U2CORE_EXPORT UniprotBlastAndLoadDASAnnotations : public Task{
     Q_OBJECT
public:
    UniprotBlastAndLoadDASAnnotations( const DASAnnotationsSettings &_settings );

    void prepare( );
    QList<Task *> onSubTaskFinished( Task *subTask );
    QList<AnnotationData> prepareResults( );
    QStringList getAccessionNumbers( ) { return dasData.getAccessionNumbers( ); }
    QStringList getProblems() {return problems;}

private:
    DASAnnotationsSettings  settings;

    UniprotBlastTask*       blastTask;
    QList<Task*>            dasTasks;

    DASAnnotationData       dasData;
    QStringList             problems;
};

}   // namespace U2

#endif // _U2_UNIPROT_API_TASK_H_
