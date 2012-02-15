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

#ifndef _WEB_TRANSPORT_PROTOCOL_H_
#define _WEB_TRANSPORT_PROTOCOL_H_

#include <stdexcept>

#include <QtCore/QByteArray>
#include <QtCore/QVariantMap>
#include <QtCore/QIODevice>
#include <QtCore/QUuid>
#include <QtCore/QBuffer>

#include <QtXml/QXmlDefaultHandler>
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlStreamWriter>



namespace U2 {

// UGENE Common Transport Protocol (UCTP) definitions

typedef QByteArray UctpCommand;

class UctpElements {
public:
    static const QByteArray REQUEST;
    static const QByteArray RESPONSE;
    static const QByteArray APP_VERSION;
    static const QByteArray STATUS;
    static const QByteArray TASK_RESULTS;
    static const QByteArray MESSAGE;
    static const QByteArray SESSION;
    static const QByteArray USER_NAME;
    static const QByteArray PASSWD;
    static const QByteArray DATA;
    static const QByteArray PROPERTY;
    static const QByteArray TASK;
    static const QByteArray TASK_STATE;
    static const QByteArray TASK_PROGRESS;
    static const QByteArray TASK_ERROR;
    static const QByteArray DATE_SUBMITTED;
};

class UctpAttributes {
public:
    static const QByteArray TASK_ID;
    static const QByteArray SESSION_ID;
    static const QByteArray COMMAND_TYPE;
    static const QByteArray IS_GZIPPED;
    static const QByteArray ENCODING;
    static const QByteArray NAME;
    static const QByteArray SIZE;
};

class UctpValues {
public:
    static const QByteArray BASE64;
    static const QByteArray STATUS_OK;
    static const QByteArray UCTP_TRUE;
    static const QByteArray UCTP_FALSE;
    static const QByteArray UCTP_STATE_FINISHED;
    static const QByteArray UCTP_STATE_ENQUED;
    static const QByteArray UCTP_STATE_RUNNING;
    static const QByteArray STATUS_ERROR;
};

class UctpCommands {
public:
    static const UctpCommand AUTH;
    static const UctpCommand RUN_TASK;
    static const UctpCommand CANCEL_TASK;
    static const UctpCommand DELETE_TASK;
    static const UctpCommand GET_TASK_RESULT;
    static const UctpCommand GET_PROPERTY;
    static const UctpCommand PING;
};

class BaseGlobalProperties {
public:
    static const QByteArray TASKS_LIST_ACTIVE;
    static const QByteArray TASKS_LIST_FINISHED;
    static const QByteArray HOST_NAME;

};

struct UctpAttribute {
    UctpAttribute() {}
    UctpAttribute(const QString& n, const QString& v) : name(n), value(v) {}
    QString name;
    QString value;
};

struct UctpElementData {
    QString textData;
    QMap<QByteArray,QByteArray> attributesMap;
};

class UctpReplyContext {
public:
    UctpReplyContext(const UctpCommand& cmd, const QList<QString>& elements )
        : command(cmd), expectedElements(elements) {}
    bool hasElement(const QString& name) { return expectedElements.contains(name); }
    const UctpCommand& getCommand() const {return command;}
    const QList<QString>& getExpectedElements() const {return expectedElements;}
private:
    UctpCommand command;
    QList<QString> expectedElements;
};

class UctpReplyHandler : public QXmlDefaultHandler {
    QString curText;
    QString errorMessage;
    QString dataEntryName;
    bool replyResultOk;
    const UctpReplyContext context;
    QMap<QString,UctpElementData>& replyData;
    QMap<QString,QXmlAttributes> xmlAttrMap;
    // validation flags present
    bool envelope, header, contents;
public:
    UctpReplyHandler(const UctpReplyContext& ctx, QMap<QString,UctpElementData>& replyData);
    bool startElement(const QString &namespaceURI, const QString &localName,
        const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
          const QString &qName);
    bool endDocument();
    bool characters(const QString &str);
    QString errorString() const;
private:
    bool validateContext();
};

class UctpRequestBuilder {
    UctpCommand command;
private:
    QBuffer requestBuf;
protected:
    virtual void formContents(QXmlStreamWriter& writer) = 0;
public:
    virtual ~UctpRequestBuilder() {}
    UctpRequestBuilder(const UctpCommand& cmd) : command(cmd) {}
    const UctpCommand& getCommand() const { return command; }
    virtual QIODevice* getDataSource();
};

class UctpSession {
public:
    UctpSession(const QByteArray& id) : uid(id) {}
    const QByteArray& getUid() const { return uid; }
    void buildQUuid(QUuid* uuid) const;
private:
    QByteArray uid;
};

class UctpError : public std::runtime_error
{
public:
    UctpError(QString message) : std::runtime_error(message.toAscii().constData()) {}
    const QString getErrorMessage() {return QString( what() ); }
    ~UctpError() throw();
private:
    QString errorMessage;
};

class Uctp {
    QXmlSimpleReader xmlReader;
    QString errorString;
public:
    bool parseReply(QIODevice* reply, const UctpCommand& command, QMap<QString,UctpElementData>& replyData);
    const QString& getErrorText() const { return errorString; } 
};





} // namespace U2

#endif // _WEB_TRANSPORT_PROTOCOL_H_
