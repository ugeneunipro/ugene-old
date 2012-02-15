/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "WebTransportProtocol.h"

#include <U2Core/Log.h>
#include <U2Core/Version.h>

namespace U2 {

const QByteArray UctpElements::REQUEST("request");
const QByteArray UctpElements::RESPONSE("response");
const QByteArray UctpElements::STATUS("status");
const QByteArray UctpElements::APP_VERSION("app-version");
const QByteArray UctpElements::MESSAGE("message");
const QByteArray UctpElements::USER_NAME("username");
const QByteArray UctpElements::PASSWD("passwd");
const QByteArray UctpElements::PROPERTY("property");
const QByteArray UctpElements::TASK("task");
const QByteArray UctpElements::TASK_STATE("state");
const QByteArray UctpElements::TASK_ERROR("error");
const QByteArray UctpElements::TASK_PROGRESS("progress");
const QByteArray UctpElements::TASK_RESULTS("task-results");
const QByteArray UctpElements::DATA("data");
const QByteArray UctpElements::DATE_SUBMITTED("state.NEW");
const QByteArray UctpElements::SESSION("session");

const QByteArray UctpAttributes::TASK_ID("task-id");
const QByteArray UctpAttributes::SESSION_ID("session-id");
const QByteArray UctpAttributes::COMMAND_TYPE("type");
const QByteArray UctpAttributes::IS_GZIPPED("gzipped");
const QByteArray UctpAttributes::ENCODING("encoding");
const QByteArray UctpAttributes::NAME("name");
const QByteArray UctpAttributes::SIZE("size");

const UctpCommand UctpCommands::AUTH("AUTH");
const UctpCommand UctpCommands::RUN_TASK("RUN_TASK");
const UctpCommand UctpCommands::DELETE_TASK("DELETE_TASK");
const UctpCommand UctpCommands::PING("PING");
const UctpCommand UctpCommands::GET_TASK_RESULT("GET_TASK_RESULT");
const UctpCommand UctpCommands::GET_PROPERTY("GET_PROPERTY");
const UctpCommand UctpCommands::CANCEL_TASK("CANCEL_TASK");

const QByteArray UctpValues::BASE64("BASE64");
const QByteArray UctpValues::UCTP_TRUE("true");
const QByteArray UctpValues::UCTP_FALSE("false");
const QByteArray UctpValues::UCTP_STATE_FINISHED("FINISHED");
const QByteArray UctpValues::STATUS_ERROR("ERROR");
const QByteArray UctpValues::STATUS_OK("OK");

const QByteArray BaseGlobalProperties::HOST_NAME("server-name");
const QByteArray BaseGlobalProperties::TASKS_LIST_ACTIVE("tasks-list-active");
const QByteArray BaseGlobalProperties::TASKS_LIST_FINISHED("tasks-list-finished");

//////////////////////////////////////////////////////////////////////////

void UctpSession::buildQUuid( QUuid* uuid ) const {
    assert(uuid != NULL);
    
    // construct uuid from MD5 representation of session-id
    
    QByteArray digest = QByteArray::fromHex(uid);
    memcpy((unsigned char*)(uuid), digest.constData(), sizeof(QUuid));
   
    assert(!uuid->isNull());
}

QIODevice* UctpRequestBuilder::getDataSource()
{
    QByteArray request;
    QXmlStreamWriter stream(&request);

    stream.writeStartDocument();
    stream.writeStartElement(UctpElements::REQUEST);
    stream.writeAttribute(UctpAttributes::COMMAND_TYPE, getCommand());
    stream.writeAttribute(UctpElements::APP_VERSION, Version::appVersion().text );

    // Contents section
    formContents(stream);

    stream.writeEndElement();
    stream.writeEndDocument();

    requestBuf.setData(request);

    return &requestBuf;
}


bool Uctp::parseReply( QIODevice* reply, const UctpCommand& command, QMap<QString,UctpElementData>& data) {
    QXmlInputSource source(reply);

    QList<QString> expectedElements; // = getExpectedElements(command);
    UctpReplyContext context(command, expectedElements);
    UctpReplyHandler* handler = new UctpReplyHandler(context,data);
    xmlReader.setContentHandler(handler);
    xmlReader.setErrorHandler(handler);
    bool ok = xmlReader.parse(source);
    errorString = handler->errorString();
    
    return ok;
}


//////////////////////////////////////////////////////////////////////////

UctpReplyHandler::UctpReplyHandler(const UctpReplyContext& ctx, QMap<QString,UctpElementData>& data) 
 : replyResultOk(false), context(ctx), replyData(data), envelope(false), header(false), contents(false)
{

}

bool UctpReplyHandler::startElement( const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes )
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    if (qName == UctpElements::RESPONSE) {
        envelope = true;
    } 
    xmlAttrMap.insert(qName, attributes);
    ioLog.trace(QString("Parsing element %1").arg(qName));
    curText.clear();
    return true;
    
}

bool UctpReplyHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName) {
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);
    
    const QXmlAttributes& currentAttributes = xmlAttrMap.value(qName);

    if (qName == UctpElements::RESPONSE) {
        QString status = currentAttributes.value(QString(UctpElements::STATUS));
        if (status == UctpValues::STATUS_OK) {
            replyResultOk = true;
            coreLog.trace("Uctp request succeed");
        } 
    } 
    
    UctpElementData data;
    data.textData = curText;
    for (int i = 0; i < currentAttributes.length(); ++i) {
        QString aName = currentAttributes.qName(i);
        QString aValue = currentAttributes.value(i);
        data.attributesMap.insert(aName.toAscii(),aValue.toAscii());
    }
    replyData.insertMulti(qName, data);
    
    return true;
}

bool UctpReplyHandler::endDocument() {
    bool structureValid  = envelope;
    if (!structureValid) {
        errorMessage = QString(QObject::tr("Invalid response structure"));
        return false;
    }
    if (!validateContext()) {
        errorMessage = QString(QObject::tr("Invalid response: missing elements"));
        return false;
    }

    if (!replyResultOk) {
        QString reason;
        if (replyData.contains(UctpElements::MESSAGE)) {
            reason = replyData.value(UctpElements::MESSAGE).textData;
        } else {
            reason = QString("status is not available");
        }
        errorMessage = QString("RService request failed: %1.").arg(reason);
        return false;
    }
    return true;
}



bool UctpReplyHandler::characters(const QString &str) {
    curText += str;
    return true;
}

bool UctpReplyHandler::validateContext() {
    foreach (const QString& elemName, context.getExpectedElements()) {
        if(!replyData.contains(elemName)) {
            return false;
        }
    }
    
    return true;
}

QString UctpReplyHandler::errorString() const {
    if (errorMessage.isEmpty()) {
        return QString("Error processing XML response");
    } else {
        return errorMessage;
    }
}


UctpError::~UctpError() throw()
{

}




} // namespace U2
