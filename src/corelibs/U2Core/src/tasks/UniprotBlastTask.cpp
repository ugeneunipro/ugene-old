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

#include "UniprotBlastTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/NetworkConfiguration.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/LoadDASDocumentTask.h>

#include <QtXml/QDomDocument>

namespace U2 {

const QByteArray ReplyHandler::LOCATION = "Location";
const QByteArray ReplyHandler::RETRY = "Retry-After";
const QByteArray ReplyHandler::CONTENT_TYPE = "Content-Type";
const QByteArray ReplyHandler::XML_CONTENT_TYPE = "application/xml";
const QByteArray ReplyHandler::COMPLETED = "COMPLETED";
const QByteArray ReplyHandler::RUNNING = "RUNNING";

ReplyHandler::ReplyHandler(const QString& _url, TaskStateInfo* _os) :
    url(_url),
    os(_os) {
    networkManager = new QNetworkAccessManager();
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(sl_replyFinished(QNetworkReply*)));
    NetworkConfiguration* nc = AppContext::getAppSettings()->getNetworkConfiguration();

    QNetworkProxy proxy = nc->getProxyByUrl(url);
    networkManager->setProxy(proxy);
}

void ReplyHandler::sendRequest() {
    QUrl requestUrl(url);
    QNetworkRequest request(requestUrl);

    QNetworkReply* reply = networkManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(sl_onError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(uploadProgress(qint64, qint64)),
            this, SLOT(sl_uploadProgress(qint64, qint64)));
}

void ReplyHandler::sl_replyFinished(QNetworkReply *reply) {
    CHECK_EXT(!os->isCoR(), emit si_finish(), );

    replyData = reply->readAll();
    ioLog.trace(QString("Server reply received, request url=\'%1\'").arg(reply->url().toString()));
    if (reply->rawHeader(CONTENT_TYPE) == XML_CONTENT_TYPE) {
        // Downloading of the result file is finished.
        // Leave the event loop.
        ioLog.trace("An xml was received");
        emit si_step(DownloadingComplete);
        emit si_finish();
    }

    else if (reply->hasRawHeader(LOCATION)) {
        // Server set an ID to the blast job.
        // Results could be found on the special URL.
        emit si_step(Ordered);

        url = reply->rawHeader(LOCATION);
        url += ".stat";
        ioLog.trace(QString("Redirection, new url: \'%1\'").arg(url));

        // Check the blast job status
        sendRequest();
    }

    else if (reply->hasRawHeader(RETRY)) {
        if (replyData == COMPLETED) {
            //The blast job is done, we can dowload a xml file.
            emit si_step(WaitingComplete);

            url.chop(QString(".stat").length());
            url += ".xml";
            ioLog.trace(QString("The blast job is complete, downloading results from \'%1\'").arg(url));
            sendRequest();
        } else if (replyData == RUNNING) {
            // The blast job is unfinished.
            // Please, wait.
            emit si_step(Waiting);

            QByteArray waitFor = reply->rawHeader(RETRY);
            bool ok = false;
            int interval = waitFor.toInt(&ok);
            if (ok == false || interval <= 0) {
                interval = 10;
            }

            ioLog.trace("Still running the blast job...");
            QTimer::singleShot(interval, this, SLOT(sl_timerShouts()));
        }
    }

    else {
        os->setError("Unexpected server responce");
        emit si_finish();
    }
}


void ReplyHandler::sl_onError(QNetworkReply::NetworkError error) {
    os->setError(QString("NetworkReply error %1").arg(error));
    emit si_finish();
}

void ReplyHandler::sl_uploadProgress(qint64 bytesSent, qint64 bytesTotal) {
    os->progress = bytesSent/ bytesTotal * 100;
}

void ReplyHandler::sl_timerShouts() {
    // Look for results, if they are.
    sendRequest();
}

const QString XmlUniprotParser::EBI_APPLICATION_RESULT = "EBIApplicationResult";
const QString XmlUniprotParser::SEQUENCE_SIMILARITY_SEARCH_RESULT = "SequenceSimilaritySearchResult";
const QString XmlUniprotParser::HITS = "hits";
const QString XmlUniprotParser::HITS_TOTAL = "total";
const QString XmlUniprotParser::HIT = "hit";
const QString XmlUniprotParser::HIT_NUMBER = "number";
const QString XmlUniprotParser::HIT_DATABASE = "database";
const QString XmlUniprotParser::HIT_ID = "id";
const QString XmlUniprotParser::HIT_AC = "ac";
const QString XmlUniprotParser::HIT_LENGTH = "length";
const QString XmlUniprotParser::HIT_DESCRIPTION = "description";
const QString XmlUniprotParser::ALIGNMENTS = "alignments";
const QString XmlUniprotParser::ALIGNMENTS_TOTAL = "total";
const QString XmlUniprotParser::ALIGNMENT = "alignment";
const QString XmlUniprotParser::ALIGNMENT_NUMBER = "number";
const QString XmlUniprotParser::SCORE = "score";
const QString XmlUniprotParser::BITS = "bits";
const QString XmlUniprotParser::EXPECTATION = "expectation";
const QString XmlUniprotParser::IDENTITY = "identity";
const QString XmlUniprotParser::POSITIVES = "positives";
const QString XmlUniprotParser::GAPS = "gaps";
const QString XmlUniprotParser::STRAND = "strand";
const QString XmlUniprotParser::QUERY_SEQ = "querySeq";
const QString XmlUniprotParser::QUERY_SEQ_START = "start";
const QString XmlUniprotParser::QUERY_SEQ_END = "end";
const QString XmlUniprotParser::PATTERN = "pattern";
const QString XmlUniprotParser::MATCH_SEQ = "matchSeq";
const QString XmlUniprotParser::MATCH_SEQ_START = "start";
const QString XmlUniprotParser::MATCH_SEQ_END = "end";

void XmlUniprotParser::parse(const QByteArray &data) {
    QString buf;        // to read int values
    bool ok = false;    // to convert strings to int

    QDomDocument pDoc;
    pDoc.setContent(data);

    QDomElement docElement = pDoc.documentElement();
    CHECK_EXT(!docElement.childNodes().isEmpty(), setError("There is no any child nodes"), );

    QDomElement sequenceSimilaritySearchResultElement = docElement.firstChildElement(SEQUENCE_SIMILARITY_SEARCH_RESULT);
    CHECK_EXT(!sequenceSimilaritySearchResultElement.isNull(), setError(QString("%1 element is NULL").arg(SEQUENCE_SIMILARITY_SEARCH_RESULT)), );

    QDomElement hitsElement = sequenceSimilaritySearchResultElement.firstChildElement(HITS);
    CHECK_EXT(!hitsElement.isNull(), setError(QString("%1 element is NULL").arg(HITS)), );

    buf = hitsElement.attribute(HITS_TOTAL);
    int hitsTotal = buf.toInt(&ok);
    CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(HITS + HITS_TOTAL).arg(buf), );

    // Parse all hits
    QDomElement hitElement = hitsElement.firstChildElement(HIT);
    CHECK_EXT(!hitElement.isNull() || hitsTotal == 0, setError(QString("%1 element is NULL").arg(HIT)), );

    for (int hit = 0;
         hit < hitsTotal;
         hit++, hitElement = hitElement.nextSiblingElement(HIT)) {
        UniprotResult result;

        CHECK_EXT(!hitElement.isNull(), setError(QString("%1 element is NULL").arg(HIT)), );

        buf = hitElement.attribute(HIT_NUMBER);
        result.hitNumber = buf.toInt(&ok);
        CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(HIT + HIT_NUMBER).arg(buf), );

        result.database = hitElement.attribute(HIT_DATABASE);
        result.id = hitElement.attribute(HIT_ID);
        result.accession = hitElement.attribute(HIT_AC);

        buf = hitElement.attribute(HIT_LENGTH);
        result.length = buf.toInt(&ok);
        CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(HIT + HIT_LENGTH).arg(buf), );

        result.description = hitElement.attribute(HIT_DESCRIPTION);

        QDomElement alignmentsElement = hitElement.firstChildElement(ALIGNMENTS);
        CHECK_EXT(!alignmentsElement.isNull(), setError(QString("%1 element is NULL").arg(ALIGNMENTS)), );

        buf = alignmentsElement.attribute(ALIGNMENTS_TOTAL);
        result.alignmentsTotal = buf.toInt(&ok);
        CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(ALIGNMENTS + ALIGNMENTS_TOTAL).arg(buf), );

        // Parse all alignments of the current hit
        QDomElement alignmentElement = alignmentsElement.firstChildElement(ALIGNMENT);
        CHECK_EXT(!alignmentElement.isNull() || result.alignmentsTotal == 0, setError(QString("%1 element is NULL").arg(ALIGNMENT)), );

        for (int alignemnt = 0;
             alignemnt < result.alignmentsTotal;
             alignemnt++, alignmentElement = alignmentElement.nextSiblingElement(ALIGNMENT)) {
            CHECK_EXT(!alignmentElement.isNull(), setError(QString("%1 element is NULL").arg(ALIGNMENT)), );

            buf = hitElement.attribute(ALIGNMENT_NUMBER);
            result.alignmentsNumber = buf.toInt(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(ALIGNMENT + ALIGNMENT_NUMBER).arg(buf), );

            QDomElement scoreElement = alignmentElement.firstChildElement(SCORE);
            CHECK_EXT(!scoreElement.isNull(), setError(QString("%1 element is NULL").arg(SCORE)), );
            buf = scoreElement.text();
            result.score = buf.toInt(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(SCORE).arg(buf), );

            QDomElement bitsElement = alignmentElement.firstChildElement(BITS);
            CHECK_EXT(!bitsElement.isNull(), setError(QString("%1 element is NULL").arg(BITS)), );
            buf = bitsElement.text();
            result.bits = buf.toDouble(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(BITS).arg(buf), );

            QDomElement expectationElement = alignmentElement.firstChildElement(EXPECTATION);
            CHECK_EXT(!expectationElement.isNull(), setError(QString("%1 element is NULL").arg(EXPECTATION)), );
            buf = expectationElement.text();
            result.expectation = buf.toDouble(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(EXPECTATION).arg(buf), );

            QDomElement identityElement = alignmentElement.firstChildElement(IDENTITY);
            CHECK_EXT(!identityElement.isNull(), setError(QString("%1 element is NULL").arg(IDENTITY)), );
            buf = identityElement.text();
            result.identity = buf.toDouble(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(IDENTITY).arg(buf), );

            QDomElement positivesElement = alignmentElement.firstChildElement(POSITIVES);
            CHECK_EXT(!positivesElement.isNull(), setError(QString("%1 element is NULL").arg(POSITIVES)), );
            buf = positivesElement.text();
            result.positives = buf.toDouble(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(POSITIVES).arg(buf), );

            QDomElement gapsElement = alignmentElement.firstChildElement(GAPS);
            CHECK_EXT(!gapsElement.isNull(), setError(QString("%1 element is NULL").arg(GAPS)), );
            buf = gapsElement.text();
            result.gaps = buf.toInt(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(GAPS).arg(buf), );

            QDomElement strandElement = alignmentElement.firstChildElement(STRAND);
            CHECK_EXT(!strandElement.isNull(), setError(QString("%1 element is NULL").arg(STRAND)), );
            result.strand = strandElement.text();

            QDomElement querySeqElement = alignmentElement.firstChildElement(QUERY_SEQ);
            CHECK_EXT(!querySeqElement.isNull(), setError(QString("%1 element is NULL").arg(QUERY_SEQ)), );
            result.querySeq = querySeqElement.text();

            buf = querySeqElement.attribute(QUERY_SEQ_START);
            result.querySeqStart = buf.toInt(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(QUERY_SEQ + QUERY_SEQ_START).arg(buf), );

            buf = querySeqElement.attribute(QUERY_SEQ_END);
            result.querySeqEnd = buf.toInt(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(QUERY_SEQ + QUERY_SEQ_END).arg(buf), );

            QDomElement patternElement = alignmentElement.firstChildElement(PATTERN);
            CHECK_EXT(!patternElement.isNull(), setError(QString("%1 element is NULL").arg(PATTERN)), );
            result.pattern = patternElement.text();

            QDomElement matchSeqElement = alignmentElement.firstChildElement(MATCH_SEQ);
            CHECK_EXT(!matchSeqElement.isNull(), setError(QString("%1 element is NULL").arg(MATCH_SEQ)), );
            result.matchSeq = matchSeqElement.text();

            buf = matchSeqElement.attribute(MATCH_SEQ_START);
            result.matchSeqStart = buf.toInt(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(MATCH_SEQ + MATCH_SEQ_START).arg(buf), );

            buf = matchSeqElement.attribute(MATCH_SEQ_END);
            result.matchSeqEnd = buf.toInt(&ok);
            CHECK_EXT(ok, QString("Can't parse the \'%1\'' value: \'%2\'").arg(MATCH_SEQ + MATCH_SEQ_END).arg(buf), );
        }

        results << result;
    }
}

const QString UniprotBlastSettings::DATABASE = "dataset";
const QString UniprotBlastSettings::THRESHOLD = "threshold";
const QString UniprotBlastSettings::MATRIX = "matrix";
const QString UniprotBlastSettings::FILTERING = "filter";
const QString UniprotBlastSettings::GAPPED = "gapped";
const QString UniprotBlastSettings::HITS = "numal";

const QString UniprotBlastSettings::DEFAULT_DATABASE = "uniprotkb";
const QString UniprotBlastSettings::DEFAULT_THRESHOLD = "10";
const QString UniprotBlastSettings::DEFAULT_MATRIX = "";
const QString UniprotBlastSettings::DEFAULT_FILTERING = "false";
const QString UniprotBlastSettings::DEFAULT_GAPPED = "true";
const QString UniprotBlastSettings::DEFAULT_HITS = "250";

const QStringList UniprotBlastSettings::ALLOWED_DATABASE = QStringList() << "uniprotkb"
                                                                         << "uniprotkb_archaea"
                                                                         << "uniprotkb_bacteria"
                                                                         << "uniprotkb_eukaryota"
                                                                         << "uniprotkb_arthropoda"
                                                                         << "uniprotkb_fungi"
                                                                         << "uniprotkb_human"
                                                                         << "uniprotkb_mammals"
                                                                         << "uniprotkb_nematoda"
                                                                         << "uniprotkb_plants"
                                                                         << "uniprotkb_rodents"
                                                                         << "uniprotkb_vertebrates"
                                                                         << "uniprotkb_viruses"
                                                                         << "uniprotkb_pdb"
                                                                         << "uniprotkb_complete_microbial_proteomes"
                                                                         << "uniprotkb_swissprot"
                                                                         << "UniRef100"
                                                                         << "UniRef90"
                                                                         << "UniRef50"
                                                                         << "uniparc";
const QStringList UniprotBlastSettings::ALLOWED_THRESHOLD = QStringList() << "0.0001"
                                                                          << "0.001"
                                                                          << "0.01"
                                                                          << "0.1"
                                                                          << "1"
                                                                          << "10"
                                                                          << "100"
                                                                          << "1000";
const QStringList UniprotBlastSettings::ALLOWED_MATRIX = QStringList() << ""                // marked as "auto"
                                                                       << "blosum45"
                                                                       << "blosum62"
                                                                       << "blosum80"
                                                                       << "pam70"
                                                                       << "pam30";
const QStringList UniprotBlastSettings::ALLOWED_FILTERING = QStringList() << "false"
                                                                          << "true"
                                                                          << "mask";
const QStringList UniprotBlastSettings::ALLOWED_GAPPED = QStringList() << "true"
                                                                       << "false";
const QStringList UniprotBlastSettings::ALLOWED_HITS = QStringList() << "50"
                                                                     << "100"
                                                                     << "250"
                                                                     << "500"
                                                                     << "750"
                                                                     << "1000";

bool UniprotBlastSettings::isValid() const {
    return ALLOWED_DATABASE.contains(settings.value(DATABASE, DEFAULT_DATABASE), Qt::CaseInsensitive) &&
            ALLOWED_THRESHOLD.contains(settings.value(THRESHOLD, DEFAULT_THRESHOLD), Qt::CaseInsensitive) &&
            ALLOWED_MATRIX.contains(settings.value(MATRIX, DEFAULT_MATRIX), Qt::CaseInsensitive) &&
            ALLOWED_FILTERING.contains(settings.value(FILTERING, DEFAULT_FILTERING), Qt::CaseInsensitive) &&
            ALLOWED_GAPPED.contains(settings.value(GAPPED, DEFAULT_GAPPED), Qt::CaseInsensitive) &&
            ALLOWED_HITS.contains(settings.value(HITS, DEFAULT_HITS), Qt::CaseInsensitive);
}

QString UniprotBlastSettings::getString() const {
    QString result;
    result += DATABASE + "=" + settings.value(DATABASE, DEFAULT_DATABASE);
    result += "&";
    result += THRESHOLD + "=" + settings.value(THRESHOLD, DEFAULT_THRESHOLD);
    result += "&";
    result += MATRIX + "=" + settings.value(MATRIX, DEFAULT_MATRIX);
    result += "&";
    result += FILTERING + "=" + settings.value(FILTERING, DEFAULT_FILTERING);
    result += "&";
    result += GAPPED + "=" + settings.value(GAPPED, DEFAULT_GAPPED);
    result += "&";
    result += HITS + "=" + settings.value(HITS, DEFAULT_HITS);
    return result;
}

const QByteArray UniprotBlastTask::BASE_URL = "http://www.uniprot.org/blast/";

UniprotBlastTask::UniprotBlastTask(const QByteArray& _sequence, const UniprotBlastSettings &_settings) :
    Task(tr("Get an ID for the sequence"), TaskFlags_FOSE_COSC | TaskFlag_MinimizeSubtaskErrorText),
    sequence(_sequence),
    settings(_settings),
    loop(NULL) {
    tpm = Progress_Manual;
}

UniprotBlastTask::~UniprotBlastTask() {
    delete loop;
}

void UniprotBlastTask::run() {
    CHECK(isCanceled() == false, );
    CHECK(hasError() == false, );

    loop = new QEventLoop;
    stateInfo.progress = 0;
    QString url = generateUrl();

    ioLog.trace("Sending request to UniProt...");
    ioLog.trace(QString("url: \'%1\'").arg(url));

    ReplyHandler replyHandler(url, &stateInfo);
    connect(&replyHandler, SIGNAL(si_finish()), SLOT(sl_exitLoop()));
    connect(&replyHandler, SIGNAL(si_step(int)), SLOT(sl_stateChanged(int)));

    // Send the first request: ask to do blast search
    replyHandler.sendRequest();

    loop->exec();
    ioLog.trace("Download finished.");

    if (isCanceled()){
        return;
    }
    
    //parse output
    QByteArray replyData = replyHandler.getReplyData();

    if (replyData.isEmpty()){
        setError(tr("Reply from the BLAST server is empty. Similar sequences not found"));
        return;
    }
    

    XmlUniprotParser parser;
    parser.parse(replyData);
    if (!parser.getError().isEmpty()) {
        setError(parser.getError());
    } else {
        results = parser.getResults();
    }
}

void UniprotBlastTask::sl_exitLoop() {
    SAFE_POINT(loop != NULL, "Main task loop is NULL", );
    loop->exit();
}

void UniprotBlastTask::sl_stateChanged(int step) {
    if (step == ReplyHandler::Ordered) {
        stateInfo.setProgress(15);
    } else if (step == ReplyHandler::Waiting) {
        stateInfo.setProgress(60);
    } else if (step == ReplyHandler::WaitingComplete) {
        stateInfo.setProgress(85);
    } else if (step == ReplyHandler::DownloadingComplete) {
        stateInfo.setProgress(100);
    } else {
        FAIL("Unexpected step value", );
    }
}

QString UniprotBlastTask::generateUrl() {
    QString url;
    CHECK_EXT(!sequence.isEmpty(), setError(tr("An empty sequence is searched")), url);
    CHECK_EXT(settings.isValid(), setError(tr("Settings are invalid")), url);

    url = BASE_URL;
    url += "?";
    url += "query=" + sequence;
    url += "&" + settings.getString();
    return url;
}

//////////////////////////////////////////////////////////////////////////
//UniprotBlastAndLoadDASAnnotations
UniprotBlastAndLoadDASAnnotations::UniprotBlastAndLoadDASAnnotations( const DASAnnotationsSettings& _settings )
:Task(tr("BLAST IDs and DAS annotations"), TaskFlags_FOSE_COSC | TaskFlag_NoRun)
,settings(_settings)
,dasData(_settings.sequence.length(), _settings.identityThreshold)
,blastTask(NULL)
{

}

void UniprotBlastAndLoadDASAnnotations::prepare(){
    blastTask = new UniprotBlastTask(settings.sequence, settings.blastSettings);
    addSubTask(blastTask);
}

static bool identityLessThan(const UniprotResult &a, const UniprotResult &b) {
    return a.identity > b.identity;
}

QList<Task*> UniprotBlastAndLoadDASAnnotations::onSubTaskFinished( Task* subTask ){
    QList<Task*> subtasks;

    if (isCanceled()){
        return subtasks;
    }
    
    if (subTask == blastTask){
        QList<UniprotResult> uniprotResults = blastTask->getResults();
        //take first results with the highest identity
        qSort(uniprotResults.begin(), uniprotResults.end(), identityLessThan);
        for(int i = 0; dasTasks.size() < settings.maxResults && i<uniprotResults.size(); i++){
            const UniprotResult& uniprotRes = uniprotResults.at(i);
            if (uniprotRes.identity >= settings.identityThreshold){
                if (uniprotRes.accession.isEmpty()){
                    continue;
                }
                
                foreach (DASSource featureSource, settings.featureSources) {
                    LoadDASObjectTask* loadAnnotationsTask = new LoadDASObjectTask(uniprotRes.accession, featureSource, DASFeatures);
                    dasTasks << loadAnnotationsTask;
                }
            }
        }
        subtasks = dasTasks;
    }else if(dasTasks.contains(subTask)){
        LoadDASObjectTask* loadDasObjectTask = qobject_cast<LoadDASObjectTask*>(subTask);
        if (!loadDasObjectTask){
            return subtasks;
        }

        dasTasks.removeAll(loadDasObjectTask);

        dasData.addDasGroup(loadDasObjectTask->getAccession(), loadDasObjectTask->getAnnotationData());
    }
    return subtasks;
}

QList<SharedAnnotationData> UniprotBlastAndLoadDASAnnotations::prepareResults(){
    return dasData.prepareResults();
}


//////////////////////////////////////////////////////////////////////////
//DASAnnotationData
QStringList DASAnnotationData::getAccessionNumbers(){
    QStringList result;

    result = dasData.keys();

    return result;
}

DASGroup DASAnnotationData::getDasGroup( const QString& accNumber ){
    DASGroup dasGroup;

    if (contains(accNumber)){
        dasGroup = dasData[accNumber];
    }
    
    return dasGroup;
}

void DASAnnotationData::addDasGroup( const QString& accNumber, DASGroup dasGroup ){
    if (!contains(accNumber)){
        dasData.insert(accNumber, dasGroup);
    }else{
        DASGroup& curData = dasData[accNumber];
        const QStringList& keys =  dasGroup.keys();
        foreach(const QString& key, keys){
            if (curData.contains(key)){
                const QList<SharedAnnotationData>& curList = curData[key];
                const QList<SharedAnnotationData>& tomergeList = dasGroup[key];
                foreach(SharedAnnotationData d, tomergeList){
                    if (!curList.contains(d)){
                        curData[key].append(d);
                    }
                }
            }else{
                curData.insert(key, dasGroup[key]);
            }
        }
    }
}

bool DASAnnotationData::contains( const QString& accessionNumber ){
    return dasData.contains(accessionNumber);
}

QList<SharedAnnotationData> DASAnnotationData::prepareResults(){
    QList<SharedAnnotationData> res;

    QStringList accessionNumbers = dasData.keys();
    foreach(const QString& key, accessionNumbers){
        const DASGroup& dasGroup = dasData.value(key);
        QStringList groupNames = dasGroup.keys();
        foreach(const QString& groupKey, groupNames){
            const QList<SharedAnnotationData>& sdata = dasGroup.value(groupKey);
            foreach (SharedAnnotationData d, sdata) {
                const U2Location& location = d->location;
                if (location->isSingleRegion() && location->regions.first() == U2_REGION_MAX) {
                    //setRegion for full region sequence
                    U2Location newLoc = location;
                    newLoc->regions.clear();
                    newLoc->regions.append(U2Region(0, seqLen));
                    d->location = location;
                }else{
                    //cut annotations with the start position out of the current sequence
                    if (location->regions.size() > 0 && location->regions.first().startPos >= seqLen){
                        continue;
                    }
                }
                res.append(d);
            }
        }
    }

    return res;
}





}   // namespace U2
