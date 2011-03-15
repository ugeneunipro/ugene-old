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

#ifndef _UCTP_REQUEST_BUILDERS_H_
#define _UCTP_REQUEST_BUILDERS_H_

#include "WebTransportProtocol.h"
#include "BufferedDataReader.h"

namespace U2 {

class InitSessionRequest : public UctpRequestBuilder 
{
    QString name;
    QString pass;
public:
    InitSessionRequest(const QString& username, const QString& passwd);
protected:
    virtual void formContents(QXmlStreamWriter& stream);
};

class PingRequest : public UctpRequestBuilder {
public:
    PingRequest() : UctpRequestBuilder(UctpCommands::PING) {}
protected:
    virtual void formContents(QXmlStreamWriter& ) {}
};

class GetGlobalPropertyRequest : public UctpRequestBuilder {
    UctpSession* session;
    QByteArray propName;
public:
    GetGlobalPropertyRequest(UctpSession* s, const QByteArray& propertyName)
        : UctpRequestBuilder(UctpCommands::GET_PROPERTY), session(s), propName(propertyName) {}
protected:
    virtual void formContents(QXmlStreamWriter& writer);
};

class GetRemoteTaskPropertyRequest : public UctpRequestBuilder {
    UctpSession* session;
    qint64 taskId;
    QStringList properties;
public:
    GetRemoteTaskPropertyRequest(UctpSession* s,  qint64 id, const QStringList& props) 
        : UctpRequestBuilder(UctpCommands::GET_PROPERTY), session(s), taskId(id), properties(props) {}
protected:
    virtual void formContents(QXmlStreamWriter& writer); 
};

class GetRemoteTaskResultRequst : public UctpRequestBuilder {
    UctpSession* session;
    qint64 taskId;
public:
    GetRemoteTaskResultRequst(UctpSession* s, qint64 id) 
        : UctpRequestBuilder(UctpCommands::GET_TASK_RESULT), session(s), taskId(id) {}
protected:
    virtual void formContents(QXmlStreamWriter& writer); 
};


class RunRemoteTaskRequest : public UctpRequestBuilder {
    UctpSession* session;
    const QByteArray& taskSchema;
    const QStringList& inputUrls;
    BufferedDataReader* reader;
    QByteArray prepareRequestTemplate();
public:
    RunRemoteTaskRequest(UctpSession* session, const QByteArray& task, 
           const QStringList& input);
    ~RunRemoteTaskRequest();
    virtual QIODevice* getDataSource();
protected:
    virtual void formContents(QXmlStreamWriter& writer);
};


class UpdateRemoteTaskRequest : public UctpRequestBuilder {
    UctpSession* session;
    qint64 taskId;
public:
    UpdateRemoteTaskRequest(const UctpCommand& cmd, UctpSession* s, qint64 id) 
        : UctpRequestBuilder(cmd), session(s), taskId(id) {}
protected:
    virtual void formContents(QXmlStreamWriter& stream); 
};



} // namespace


#endif // _UCTP_REQUEST_BUILDERS_H_