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

#ifndef _U2_WORKFLOW_TRANSPORT_H_
#define _U2_WORKFLOW_TRANSPORT_H_

#include <QtCore/QVariant>

#include <U2Lang/Datatype.h>

namespace U2 {

namespace Workflow {

/**
 * Message is entity for data that is passed between actors in runtime
 */
class U2LANG_EXPORT Message {
public:
    Message(DataTypePtr _t, const QVariant& d);
    
    int getId() const;
    DataTypePtr getType() const;
    QVariant getData() const;
    
private:
    static int nextid();
    
private:
    // message identifier
    int id;
    // type of data
    DataTypePtr t;
    // data itself
    QVariant data;
    
}; // Message

/**
 * pure virtual controller of passing/receiving messages
 */
class U2LANG_EXPORT CommunicationChannel {
public:
    virtual ~CommunicationChannel() {}
    
    // take message from channel
    virtual Message get() = 0;
    // look at message without getting it out of channel
    // Message is united of data that was putted to outer channels
    // used in scripting
    virtual Message look() const = 0;
    // after calling message is in channel until get() invocation
    virtual void put(const Message& m) = 0;
    // how many messages in channel
    virtual int hasMessage() const = 0;
    // how many messages taken from channel
    virtual int takenMessages() const = 0;
    // 
    virtual int hasRoom(const DataType* t = NULL) const = 0;
    // user can set 'ended' flag to channel
    // it means that no other data will be supplied to it
    virtual bool isEnded() const = 0;
    virtual void setEnded() = 0;
    
    // how many messages can be put to channel
    virtual int capacity() const = 0;
    virtual void setCapacity(int) = 0;
    
}; // CommunicationChannel

/**
 * represents entity that communicate with others by CommunicationChannel
 * moreover, it binds channels to port identifiers
 */
class U2LANG_EXPORT CommunicationSubject {
public:
    virtual ~CommunicationSubject() {}
    
    virtual bool addCommunication(const QString& portId, CommunicationChannel* ch) = 0;
    virtual CommunicationChannel* getCommunication(const QString& portId) = 0;
    
}; // CommunicationSubject

}//Workflow namespace

}//GB2 namespace

#endif // _U2_WORKFLOW_TRANSPORT_H_
