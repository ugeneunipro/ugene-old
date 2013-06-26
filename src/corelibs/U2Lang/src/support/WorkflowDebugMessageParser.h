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

#ifndef _U2_WORKFLOW_DEBUG_MESSAGE_PARSER_H_
#define _U2_WORKFLOW_DEBUG_MESSAGE_PARSER_H_

#include "WorkflowInvestigationData.h"

namespace U2 {

using namespace Workflow;

class Workflow::Message;
class Workflow::WorkflowContext;
class BaseMessageTranslator;
class GObject;

class WorkflowDebugMessageParser {
public:
    WorkflowDebugMessageParser(const QQueue<Message> &initSource, WorkflowContext *initContext);
    ~WorkflowDebugMessageParser();

    WorkflowInvestigationData getAllMessageValues();
    void convertMessagesToDocuments(const QString &convertedType, const QString &schemeName,
        quint32 messageNumber);

private:
    void initParsedInfo();
    QString convertToString(const QString &contentIdentifier, const QVariant &content) const;
    QString getMessageTypeFromIdentifier(const QString &messageIdentifier) const;
    BaseMessageTranslator *createMessageTranslator(const QString &messageType,
        const QVariant &messageData, WorkflowContext *context) const;
    GObject *fetchObjectFromMessage(const QString &messageType, const QVariant &messageData,
        WorkflowContext *context) const;

    QQueue<QVariantMap> sourceMessages;
    QStringList messageTypes;
    WorkflowInvestigationData parsedInfo;
    WorkflowContext *context;

    static QStringList possibleMessageTypes;
};

} // namespace U2

#endif // _U2_WORKFLOW_DEBUG_MESSAGE_PARSER_H_