#include "PicrApiTask.h"

#include "LoadDASDocumentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/GObjectRelationRoles.h>

#include <QtXml/QDomDocument>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
//PicrElement
PicrElement::PicrElement(const QString& _accessionNumber,
            const int _accessionVersion,
            const QString& _databaseName,
            const QString& _databaseDescription,
            const int _ncbiId,
            const int _taxonId,
            int _identity):
    accessionNumber(_accessionNumber),
    accessionVersion(_accessionVersion),
    databaseName(_databaseName),
    databaseDescription(_databaseDescription),
    ncbiId(_ncbiId),
    taxonId(_taxonId),
    identity(_identity) {
}

bool PicrElement::operator == (const PicrElement& other) const {
    return accessionNumber == other.accessionNumber;
}

//////////////////////////////////////////////////////////////////////////
//XmlPicrIdsParser
const QString XmlPicrParser::PICR_ACCESSION_RETURN = "ns2:getUPIForAccessionReturn";
const QString XmlPicrParser::PICR_SEQUENCE_RETURN = "ns2:getUPIForSequenceReturn";
const QString XmlPicrParser::PICR_BLAST_RETURN = "ns2:getUPIForBlastReturn";
const QString XmlPicrParser::PICR_SEQUENCE = "sequence";
const QString XmlPicrParser::PICR_IDENTICAL_CROSS_REFERENCE = "identicalCrossReferences";
const QString XmlPicrParser::PICR_ACCESSION_NUMBER = "accession";
const QString XmlPicrParser::PICR_ACCESSION_VERSION = "accessionVersion";
const QString XmlPicrParser::PICR_DATABASE_DESCRIPTION = "databaseDescription";
const QString XmlPicrParser::PICR_DATABASE_NAME = "databaseName";
const QString XmlPicrParser::PICR_NCBI_GI = "gi";
const QString XmlPicrParser::PICR_TAXON_ID = "taxonId";
const QString XmlPicrParser::SERVER_ERROR_STRING = "500 internal server error";

XmlPicrParser::XmlPicrParser(ResponseType _responseType) :
    responseType(_responseType) {
}

void XmlPicrParser::parse(const QByteArray& data) {
    if (QString(data).contains(SERVER_ERROR_STRING)) {
        setError(QString("Something gone wrong on the server (%1). Please, try later.").arg(SERVER_ERROR_STRING));
        return;
    }

    QDomDocument pDoc;
    pDoc.setContent(data);

    QDomElement docElement = pDoc.documentElement();
    if (docElement.childNodes().isEmpty()) {
        setError("No IDs found.");
        return;
    }

    QDomNodeList returnList;
    QDomElement returnElement;
    if (responseType == Accession) {
        // Find the longest matching sequence
        returnList = docElement.elementsByTagName(PICR_ACCESSION_RETURN);
        int maxLength = 0;
        int maxIndex = 0;
        for (int i = 0; i < returnList.count(); ++i) {
            QDomNodeList sequenceNodeList = returnList.item(i).toElement().elementsByTagName(PICR_SEQUENCE);
            if (1 != sequenceNodeList.count()) {
                setError(QString("Invalid XML: %1 sequences found in the \"%2\" part").
                         arg(sequenceNodeList.count()).
                         arg(PICR_ACCESSION_RETURN));
                return;
            }
            QDomElement sequenceElement = sequenceNodeList.item(0).toElement();

            if (maxLength < sequenceElement.text().length()) {
                maxLength = sequenceElement.text().length();
                maxIndex = i;
            }
        }

        returnElement = returnList.item(maxIndex).toElement();
        if (returnElement.isNull()) {
            setError(QString("Invalid XML: \"%1\" element is not found").arg(PICR_ACCESSION_RETURN));
            return;
        }
    }
    else if (responseType == ExactSequence) {
        // Find the longest matching sequence
        returnList = docElement.elementsByTagName(PICR_SEQUENCE_RETURN);
        int maxLength = 0;
        int maxIndex = 0;
        for (int i = 0; i < returnList.count(); ++i) {
            QDomNodeList sequenceNodeList = returnList.item(i).toElement().elementsByTagName(PICR_SEQUENCE);
            if (1 != sequenceNodeList.count()) {
                setError(QString("Invalid XML: %1 sequences found in the \"%2\" part").
                         arg(sequenceNodeList.count()).
                         arg(PICR_SEQUENCE_RETURN));
                return;
            }
            QDomElement sequenceElement = sequenceNodeList.item(0).toElement();

            if (maxLength < sequenceElement.text().length()) {
                maxLength = sequenceElement.text().length();
                maxIndex = i;
            }
        }

        returnElement = returnList.item(maxIndex).toElement();
        returnElement = returnList.item(maxIndex).toElement();
        if (returnElement.isNull()) {
            setError(QString("Invalid XML: \"%1\" element is not found").arg(PICR_SEQUENCE_RETURN));
            return;
        }
    }
    else if (responseType == Blast) {
        returnElement = docElement;
    }

    QDomNodeList icrList = returnElement.elementsByTagName(PICR_IDENTICAL_CROSS_REFERENCE);
    if (icrList.isEmpty()) {
        setError(QString("No %1 tag").arg(PICR_IDENTICAL_CROSS_REFERENCE));
        return;
    }

    for (int i = 0; i < icrList.count(); ++i) {
        QDomNode icr = icrList.item(i);
        PicrElement element;

        for (int j = 0; j < icr.childNodes().count(); j++) {
            QDomElement childElement = icr.childNodes().item(j).toElement();
            if (childElement.tagName() == PICR_ACCESSION_NUMBER) {
                element.accessionNumber = childElement.text();
            }
            if (childElement.tagName() == PICR_ACCESSION_VERSION) {
                bool ok = false;
                int value = childElement.text().toInt(&ok);
                if (ok) {
                    element.accessionVersion = value;
                }
            }
            if (childElement.tagName() == PICR_DATABASE_DESCRIPTION) {
                element.databaseDescription = childElement.text();
            }
            if (childElement.tagName() == PICR_DATABASE_NAME) {
                element.databaseName = childElement.text();
            }
            if (childElement.tagName() == PICR_NCBI_GI) {
                bool ok = false;
                int value = childElement.text().toInt(&ok);
                if (ok) {
                    element.ncbiId = value;
                }
            }
            if (childElement.tagName() == PICR_TAXON_ID) {
                bool ok = false;
                int value = childElement.text().toInt(&ok);
                if (ok) {
                    element.taxonId = value;
                }
            }
        }

        // Sometimes the "accession" element inculdes accession version. It should be splitted.
        if (element.accessionVersion < 0 &&
                (element.accessionNumber.contains(QRegExp("\\.|\\-")))) {
            QStringList accession = element.accessionNumber.split(QRegExp("\\.|\\-"));
            SAFE_POINT(accession.count() > 1, "Invalid accession field in the XML response", );

            element.accessionNumber = accession.takeFirst();
            bool ok = false;
            int value = accession.first().toInt(&ok);
            if (ok) {
                element.accessionVersion = value;
            }
        }

        // Elements from different bases with similar ID are skipped!
        if (!results.contains(element)) {
            results << element;
        }
    }

    return;
}

QString XmlPicrParser::getError() const {
    return error;
}

void XmlPicrParser::setError(const QString& val) {
    error = val;
}

QList<PicrElement> XmlPicrParser::getResults() {
    return results;
}

bool XmlPicrParser::isResultsEmpty() {
    return results.isEmpty();
}

//////////////////////////////////////////////////////////////////////////
//ConvertDasIdTask
const QString ConvertDasIdTask::baseUrl = QString("http://www.ebi.ac.uk/Tools/picr/rest/getUPIForAccession");
const QString ConvertDasIdTask::databasePart = QString("&database=SWISSPROT&database=SWISSPROT_ID&database=SWISSPROT_VARSPLIC");
const QString ConvertDasIdTask::parametersPart = QString("&includeattributes=true");

ConvertDasIdTask::ConvertDasIdTask(const QString& resId) :
    Task(tr("Convert resource id: %1").arg(resId), TaskFlags_FOSCOE | TaskFlag_MinimizeSubtaskErrorText),
    resourceId(resId),
    loop(NULL),
    downloadReply(NULL),
    networkManager(NULL) {
}

ConvertDasIdTask::~ConvertDasIdTask() {
    delete loop;
    delete networkManager;
}

void ConvertDasIdTask::run() {
    if (stateInfo.isCanceled()) {
        return;
    }
    stateInfo.progress = 0;
    ioLog.trace("Sending request to PICR...");

    loop = new QEventLoop;

    networkManager = new QNetworkAccessManager();
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(sl_replyFinished(QNetworkReply*)));
    NetworkConfiguration* nc = AppContext::getAppSettings()->getNetworkConfiguration();

    ioLog.trace("Downloading xml file...");

    QString fetchUrl = getRequestUrlString();
    QNetworkProxy proxy = nc->getProxyByUrl(fetchUrl);
    networkManager->setProxy(proxy);
    ioLog.trace(fetchUrl);

    QUrl requestUrl(fetchUrl);
    downloadReply = networkManager->get(QNetworkRequest(requestUrl));
    connect(downloadReply, SIGNAL(error(QNetworkReply::NetworkError)),
        this, SLOT(sl_onError(QNetworkReply::NetworkError)));
    connect( downloadReply, SIGNAL(uploadProgress( qint64, qint64 )),
        this, SLOT(sl_uploadProgress(qint64,qint64)) );

    loop->exec();
    ioLog.trace("Download finished.");

    QByteArray result = downloadReply->readAll();

    //parse output
    XmlPicrParser parser(XmlPicrParser::Accession);
    parser.parse(result);
    if (!parser.getError().isEmpty()) {
        setError(parser.getError());
    } else {
        if (!parser.isResultsEmpty()) {
            // The first accession number is taken
            accNumber = parser.getResults()[0].accessionNumber;
        }
    }
}

QString ConvertDasIdTask::getAccessionNumber() {
    return accNumber;
}

void ConvertDasIdTask::sl_replyFinished(QNetworkReply* reply) {
    Q_UNUSED(reply);
    loop->exit();
}

void ConvertDasIdTask::sl_onError(QNetworkReply::NetworkError error) {
    stateInfo.setError(QString("NetworkReply error %1").arg(error));
    loop->exit();
}

void ConvertDasIdTask::sl_uploadProgress(qint64 bytesSent, qint64 bytesTotal) {
    stateInfo.progress = bytesSent/ bytesTotal * 100;
}

QString ConvertDasIdTask::getRequestUrlString() {
    QString res = "";
    if (resourceId.isEmpty()) {
        return res;
    }

    res = baseUrl + "?accession=" + resourceId + databasePart + parametersPart;

    return res;
}

//////////////////////////////////////////////////////////////////////////
//GetDasIdsBySequenceTask
GetDasIdsBySequenceTask::GetDasIdsBySequenceTask(const QByteArray& _sequence) :
    Task(tr("Get id for sequence"), TaskFlags_FOSCOE | TaskFlag_MinimizeSubtaskErrorText),
    sequence(_sequence),
    loop(NULL),
    downloadReply(NULL),
    networkManager(NULL) {
}

GetDasIdsBySequenceTask::~GetDasIdsBySequenceTask() {
    delete loop;
    delete networkManager;
}

void GetDasIdsBySequenceTask::run() {
    if (stateInfo.isCanceled()) {
        return;
    }
    stateInfo.progress = 0;
    ioLog.trace("Sending request to PICR...");

    loop = new QEventLoop;

    networkManager = new QNetworkAccessManager();
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(sl_replyFinished(QNetworkReply*)));
    NetworkConfiguration* nc = AppContext::getAppSettings()->getNetworkConfiguration();

    ioLog.trace("Downloading xml file...");

    QString fetchUrl = getRequestUrlString();
    SAFE_POINT(!fetchUrl.isEmpty(), QString("Invalid fetch URL: %1").arg(fetchUrl), );
    QNetworkProxy proxy = nc->getProxyByUrl(fetchUrl);
    networkManager->setProxy(proxy);
    ioLog.trace(fetchUrl);

    QUrl requestUrl(fetchUrl);
    downloadReply = networkManager->get(QNetworkRequest(requestUrl));
    connect(downloadReply, SIGNAL(error(QNetworkReply::NetworkError)),
        this, SLOT(sl_onError(QNetworkReply::NetworkError)));
    connect( downloadReply, SIGNAL(uploadProgress( qint64, qint64 )),
        this, SLOT(sl_uploadProgress(qint64,qint64)) );

    loop->exec();
    ioLog.trace("Download finished.");

    QByteArray result = downloadReply->readAll();

    //parse output
    XmlPicrParser parser = getParser();
    parser.parse(result);
    if (!parser.getError().isEmpty()) {
        setError(parser.getError());
    } else {
        results = parser.getResults();
    }
}

QList<PicrElement> GetDasIdsBySequenceTask::getResults() {
    return results;
}

void GetDasIdsBySequenceTask::sl_replyFinished(QNetworkReply* reply) {
    Q_UNUSED(reply);
    loop->exit();
}

void GetDasIdsBySequenceTask::sl_onError(QNetworkReply::NetworkError error) {
    stateInfo.setError(QString("NetworkReply error %1").arg(error));
    loop->exit();
}

void GetDasIdsBySequenceTask::sl_uploadProgress(qint64 bytesSent, qint64 bytesTotal) {
    stateInfo.progress = bytesSent/ bytesTotal * 100;
}

//////////////////////////////////////////////////////////////////////////
//GetDasIdsByExactSequenceTask
const QString GetDasIdsByExactSequenceTask::baseUrl = QString("http://www.ebi.ac.uk/Tools/picr/rest/getUPIForSequence");
const QString GetDasIdsByExactSequenceTask::databasePart = QString("&database=SWISSPROT&database=SWISSPROT_ID&database=SWISSPROT_VARSPLIC");
const QString GetDasIdsByExactSequenceTask::parametersPart = QString("&includeattributes=true");

GetDasIdsByExactSequenceTask::GetDasIdsByExactSequenceTask(const QByteArray& _sequence) :
    GetDasIdsBySequenceTask(_sequence) {
}

GetDasIdsByExactSequenceTask::~GetDasIdsByExactSequenceTask() {
}

QString GetDasIdsByExactSequenceTask::getRequestUrlString() {
    QString res;
    if (sequence.isEmpty()) {
        return res;
    }

    res = baseUrl + "?sequence=" + sequence + databasePart + parametersPart;
    return res;
}

XmlPicrParser GetDasIdsByExactSequenceTask::getParser() {
    return XmlPicrParser(XmlPicrParser::ExactSequence);
}

//////////////////////////////////////////////////////////////////////////
//PicrBlastSettings
/*const QStringList PicrBlastSettings::Program = QStringList() << "blastp"    //default value
                                                             << "blastx"
                                                             << "blastn"
                                                             << "tblastx"
                                                             << "tblastn";
const QStringList PicrBlastSettings::Matrix = QStringList() << "BLOSUM45"
                                                            << "BLOSUM50"
                                                            << "BLOSUM62"   // default value
                                                            << "BLOSUM80"
                                                            << "BLOSUM90"
                                                            << "PAM30"
                                                            << "PAM70"
                                                            << "PAM250";
const QList<int> PicrBlastSettings::Dropoff = QList<int>() << 0             // default value
                                                           << 2
                                                           << 4
                                                           << 6
                                                           << 8
                                                           << 10;
const QList<int> PicrBlastSettings::Align = QList<int>() << 0               // default value
                                                         << 1
                                                         << 2
                                                         << 3
                                                         << 4
                                                         << 5;
const QStringList PicrBlastSettings::Filter = QStringList() << "F"          // default value
                                                            << "T";
const QList<int> PicrBlastSettings::GapExt = QList<int>() << 1               // default value
                                                         << 1
                                                         << 2
                                                         << 3
                                                         << 4
                                                         << 5;*/

//////////////////////////////////////////////////////////////////////////
//GetDasIdsByBlastTask
const QString GetDasIdsByBlastTask::baseUrl = QString("http://www.ebi.ac.uk/Tools/picr/rest/getUPIForBLAST");
const QString GetDasIdsByBlastTask::databasePart = QString("&database=SWISSPROT&database=SWISSPROT_ID&database=SWISSPROT_VARSPLIC");
const QString GetDasIdsByBlastTask::parametersPart = QString("&filtertype=IDENTITY&identityvalue=%1&includeattributes=true");

GetDasIdsByBlastTask::GetDasIdsByBlastTask(const QByteArray& _sequence, const int _identityValue) :
    GetDasIdsBySequenceTask(_sequence),
    identityValue(_identityValue) {
}

GetDasIdsByBlastTask::~GetDasIdsByBlastTask() {
}

QString GetDasIdsByBlastTask::getRequestUrlString() {
    QString res;
    if (sequence.isEmpty()) {
        return res;
    }

    res = baseUrl + "?blastfrag=" + sequence + databasePart + parametersPart.arg(identityValue);
    return res;
}

XmlPicrParser GetDasIdsByBlastTask::getParser() {
    return XmlPicrParser(XmlPicrParser::Blast);
}

}   // namespace
