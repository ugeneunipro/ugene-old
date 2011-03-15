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

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <U2Core/GUrl.h>

#include "UctpRequestBuilders.h"
#include "Base64File.h"

namespace U2 {



InitSessionRequest::InitSessionRequest( const QString& username, const QString& passwd )
: UctpRequestBuilder(UctpCommands::AUTH), name(username), pass(passwd)
{

}

void InitSessionRequest::formContents( QXmlStreamWriter& stream )
{
    stream.writeStartElement(UctpElements::SESSION);
    stream.writeAttribute(UctpElements::USER_NAME, name);
    stream.writeAttribute(UctpElements::PASSWD, pass);
    stream.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////



#define TRANSITION_MARKER "0000_1111____TRANSITION____MARKER____1111____0000"


static qint64 getFileSize(const QString& url) {
    QFileInfo info(url);
    return info.size();
}


RunRemoteTaskRequest::RunRemoteTaskRequest( UctpSession* s, const QByteArray& ts, const QStringList& input )
 : UctpRequestBuilder(UctpCommands::RUN_TASK), session(s), taskSchema(ts), inputUrls(input)
{
    QByteArray requestTemplate = prepareRequestTemplate();
    reader = new BufferedDataReader(input, requestTemplate, TRANSITION_MARKER);
}


void RunRemoteTaskRequest::formContents( QXmlStreamWriter& stream )
{
    
    stream.writeStartDocument();
    stream.writeStartElement(UctpElements::REQUEST);
    stream.writeAttribute(UctpAttributes::COMMAND_TYPE, UctpCommands::RUN_TASK);
    stream.writeAttribute(UctpElements::APP_VERSION, Uctp::APP_VERSION );
    stream.writeAttribute(UctpAttributes::SESSION_ID, session->getUid());

    // save schema
    stream.writeStartElement(UctpElements::TASK);
    stream.writeAttribute(UctpAttributes::ENCODING, UctpValues::BASE64);
    stream.writeCharacters(taskSchema.toBase64());
    stream.writeEndElement(); 

    // save data
    foreach(const QString url, inputUrls) {
        stream.writeStartElement(UctpElements::DATA);
        stream.writeAttribute(UctpAttributes::IS_GZIPPED, UctpValues::UCTP_FALSE);
        stream.writeAttribute(UctpAttributes::ENCODING, UctpValues::BASE64);
        stream.writeAttribute(UctpAttributes::NAME, GUrl(url).fileName());
        stream.writeAttribute(UctpAttributes::SIZE, QString("%1").arg(getFileSize(url)));
        stream.writeCharacters(TRANSITION_MARKER);
        stream.writeEndElement();
    }

    stream.writeEndElement();
    stream.writeEndDocument();

}

QByteArray RunRemoteTaskRequest::prepareRequestTemplate()
{
    QByteArray requestTemplate;
    QXmlStreamWriter stream(&requestTemplate);
    
    formContents(stream);

    return requestTemplate;
}

RunRemoteTaskRequest::~RunRemoteTaskRequest()
{
    delete reader;
    reader = NULL;
}

QIODevice* RunRemoteTaskRequest::getDataSource()
{
    reader->open(QIODevice::ReadOnly);
    return reader;
}

 

//////////////////////////////////////////////////////////////////////////


void GetRemoteTaskPropertyRequest::formContents( QXmlStreamWriter& stream )
{
    stream.writeAttribute(UctpAttributes::SESSION_ID, session->getUid()); 
    
    QString id = QString("%1").arg(taskId);
    //this is task property
    stream.writeStartElement(UctpElements::PROPERTY);
    stream.writeAttribute(UctpAttributes::TASK_ID, id);
    QString propRequest = properties.join(",");
    stream.writeAttribute(UctpAttributes::NAME, propRequest);
    stream.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////


void UpdateRemoteTaskRequest::formContents( QXmlStreamWriter& stream )
{
    stream.writeAttribute(UctpAttributes::SESSION_ID, session->getUid()); 
    QString id = QString("%1").arg(taskId);
    stream.writeStartElement(UctpElements::TASK);
    stream.writeAttribute(UctpAttributes::TASK_ID, id);
    stream.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////

void GetGlobalPropertyRequest::formContents( QXmlStreamWriter& stream )
{
    stream.writeAttribute(UctpAttributes::SESSION_ID, session->getUid()); 
    stream.writeStartElement(UctpElements::PROPERTY);
    stream.writeAttribute(UctpAttributes::NAME, propName);
    stream.writeEndElement();
}


void GetRemoteTaskResultRequst::formContents( QXmlStreamWriter& stream )
{
    stream.writeAttribute(UctpAttributes::SESSION_ID, session->getUid());
    stream.writeStartElement(UctpElements::TASK);
    stream.writeAttribute(UctpAttributes::TASK_ID, QString("%1").arg(taskId));
    stream.writeEndElement();
}

} // ~namespace
