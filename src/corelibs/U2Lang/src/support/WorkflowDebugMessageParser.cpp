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

#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/TextObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Gui/ProjectView.h>
#include <U2Lang/Descriptor.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/WorkflowContext.h>

#include <U2Lang/WorkflowTransport.h>

#include "BaseMessageTranslator.h"
#include "SequenceMessageTranslator.h"
#include "AnnotationsMessageTranslator.h"
#include "MultipleAlignmentMessageTranslator.h"
#include "AssemblyMessageTranslator.h"
#include "VariationTrackMessageTranslator.h"

#include "WorkflowDebugMessageParser.h"

const QString PRODUCING_ACTOR_AND_DATA_TYPE_SEPARATOR = ":";
const QString FILE_NAME_WORDS_SEPARATOR = "_";
const quint8 MAXIMUM_COUNT_OF_LOADED_DOCUMENTS = 10;
const QString MESSAGE_PATH_DELIMETER = ">";
const QString INVESTIGATION_FILES_DIR_NAME = "investigation_files";

namespace U2 {

QStringList WorkflowDebugMessageParser::possibleMessageTypes = QStringList();

WorkflowDebugMessageParser::WorkflowDebugMessageParser(const QQueue<Message> &initSource,
    WorkflowContext *initContext) : sourceMessages(), messageTypes(), parsedInfo(),
    context(initContext)
{
    Q_ASSERT(NULL != context);
    if(possibleMessageTypes.isEmpty()) {
        possibleMessageTypes << BaseSlots::DNA_SEQUENCE_SLOT().getId();
        possibleMessageTypes << BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId();
        possibleMessageTypes << BaseSlots::ANNOTATION_TABLE_SLOT().getId();
        possibleMessageTypes << BaseSlots::TEXT_SLOT().getId();
        possibleMessageTypes << BaseSlots::URL_SLOT().getId();
        possibleMessageTypes << BaseSlots::FASTA_HEADER_SLOT().getId();
        possibleMessageTypes << BaseSlots::VARIATION_TRACK_SLOT().getId();
        possibleMessageTypes << BaseSlots::ASSEMBLY_SLOT().getId();
        possibleMessageTypes << BaseSlots::DATASET_SLOT().getId();
    }
    foreach(Message message, initSource) {
        QVariantMap data = message.getData().toMap();
        foreach(QString key, data.keys()) {
            if(key.contains(MESSAGE_PATH_DELIMETER)) {
                data[key.left(key.indexOf(MESSAGE_PATH_DELIMETER))] = data[key];
                data.remove(key);
            }
        }
        sourceMessages.enqueue(data);
    }
}

WorkflowDebugMessageParser::~WorkflowDebugMessageParser(){

}

void WorkflowDebugMessageParser::initParsedInfo() {
    if(!messageTypes.isEmpty()) {
        foreach(const QString &typeName, messageTypes) {
            parsedInfo[typeName] = QQueue<QString>();
        }
    }
}

QString WorkflowDebugMessageParser::convertToString(const QString &contentIdentifier,
    const QVariant &content) const
{
    BaseMessageTranslator *messageTranslator = createMessageTranslator(
        getMessageTypeFromIdentifier(contentIdentifier), content, context);
    Q_ASSERT(NULL != messageTranslator);
    QString result = messageTranslator->getTranslation();
    delete messageTranslator;
    return result;
}

QString WorkflowDebugMessageParser::getMessageTypeFromIdentifier(const QString &messageIdentifier)
    const
{
    return messageIdentifier.right(messageIdentifier.size() - messageIdentifier.lastIndexOf(
        PRODUCING_ACTOR_AND_DATA_TYPE_SEPARATOR) - 1);
}

WorkflowInvestigationData WorkflowDebugMessageParser::getAllMessageValues() {
    if(!sourceMessages.isEmpty()) {
        foreach(const QString &key, sourceMessages.head().keys()) {
            const QString messageType = getMessageTypeFromIdentifier(key);
            if (!possibleMessageTypes.contains(messageType)) {
                coreLog.info( QObject::tr( "Messages in requested queue include info of the '%1' "
                    "data type that is currently unsupported for view. "
                    "No intermediate data will be displayed" ).arg( messageType ) );
                return parsedInfo;
            }
            if(!messageTypes.contains(key)) {
                messageTypes << key;
            }
        }
        initParsedInfo();
        foreach(QVariantMap messageContent, sourceMessages) {
            foreach(const QString &key, messageContent.keys()) {
                Q_ASSERT(messageTypes.contains(key));
                parsedInfo[key].enqueue(convertToString(key, messageContent[key]));
            }
        }
    }
    return parsedInfo;
}

void WorkflowDebugMessageParser::convertMessagesToDocuments(const QString &convertedType,
    const QString &schemeName, quint32 messageNumber)
{
    Q_ASSERT(!convertedType.isEmpty());
    QString tmpFolderUrl
        = (AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath());
    tmpFolderUrl.replace("//", "/");

    quint32 messageCounter = ++messageNumber;
    ProjectView *projectView = AppContext::getProjectView();
    foreach(QVariantMap mapData, sourceMessages) {
        Q_ASSERT(mapData.keys().contains(convertedType));
        const QString messageType = getMessageTypeFromIdentifier(convertedType);
        const QString baseFileUrl = tmpFolderUrl + "/" + schemeName
            + FILE_NAME_WORDS_SEPARATOR + messageType + FILE_NAME_WORDS_SEPARATOR
            + "m" + QString::number(messageCounter);
        if(BaseSlots::ANNOTATION_TABLE_SLOT().getId() == messageType) {
            const QVariant annotationsData = mapData[convertedType];
            Q_ASSERT(annotationsData.canConvert<QList<SharedAnnotationData> >());
            QList<SharedAnnotationData> annotations = annotationsData
                .value<QList<SharedAnnotationData> >();
            QList<Annotation *> request;
            foreach(SharedAnnotationData annotation, annotations) {
                request << new Annotation(annotation);
            }
            projectView->exportAnnotations(request, baseFileUrl);
            qDeleteAll(request);
        } else {
            GObject *objectToWrite = fetchObjectFromMessage(messageType, mapData[convertedType],
                context);
            if(NULL != objectToWrite) {
                projectView->exportObject2Document(objectToWrite, baseFileUrl, false);
                ++messageCounter;
            }
        }
    }
}

BaseMessageTranslator *WorkflowDebugMessageParser::createMessageTranslator(
    const QString &messageType, const QVariant &messageData, WorkflowContext *context) const
{
    BaseMessageTranslator *result = NULL;
    if(BaseSlots::DNA_SEQUENCE_SLOT().getId() == messageType) {
        result = new SequenceMessageTranslator(messageData, context);
    } else if(BaseSlots::ANNOTATION_TABLE_SLOT().getId() == messageType) {
        result = new AnnotationsMessageTranslator(messageData, context);
    } else if(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId() == messageType) {
        result = new MultipleAlignmentMessageTranslator(messageData, context);
    } else if(BaseSlots::ASSEMBLY_SLOT().getId() == messageType) {
        result = new AssemblyMessageTranslator(messageData, context);
    } else if(BaseSlots::VARIATION_TRACK_SLOT().getId() == messageType) {
        result = new VariationTrackMessageTranslator(messageData, context);
    } else if(BaseSlots::TEXT_SLOT().getId() == messageType || BaseSlots::URL_SLOT().getId()
        == messageType || BaseSlots::DATASET_SLOT().getId() == messageType
        || BaseSlots::FASTA_HEADER_SLOT().getId() == messageType)
    {
        result = new BaseMessageTranslator(messageData, context);
    }
    return result;
}

GObject *WorkflowDebugMessageParser::fetchObjectFromMessage(const QString &messageType,
    const QVariant &messageData, WorkflowContext *context) const
{
    GObject *result = NULL;
    if(BaseSlots::TEXT_SLOT().getId() == messageType) {
        SAFE_POINT(messageData.canConvert<QString>(), "Supplied message doesn't contain text data",
            NULL);
        const QString documentText = messageData.value<QString>();
        result = new TextObject(documentText, "wd_investigation_tmp_text_object");
        return result;
    } else if(BaseSlots::URL_SLOT().getId() == messageType
        || BaseSlots::DATASET_SLOT().getId() == messageType
        || BaseSlots::FASTA_HEADER_SLOT().getId() == messageType
        || BaseSlots::ANNOTATION_TABLE_SLOT().getId() == messageType)
    {
        return result;
    }
    SAFE_POINT(messageData.canConvert<SharedDbiDataHandler>(),
        "Supplied message doesn't contain DB reference", NULL);
    SharedDbiDataHandler objectId = messageData.value<SharedDbiDataHandler>();
    
    if(BaseSlots::DNA_SEQUENCE_SLOT().getId() == messageType) {
        result = StorageUtils::getSequenceObject(context->getDataStorage(), objectId);
    } else if(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId() == messageType) {
        result = StorageUtils::getMsaObject(context->getDataStorage(), objectId);
    } else if(BaseSlots::ASSEMBLY_SLOT().getId() == messageType) {
        result = StorageUtils::getAssemblyObject(context->getDataStorage(), objectId);
    } else if(BaseSlots::VARIATION_TRACK_SLOT().getId() == messageType) {
        result = StorageUtils::getVariantTrackObject(context->getDataStorage(), objectId);
    }
    SAFE_POINT(NULL != result, "Could not obtain object from dbi", NULL);
    return result;
}

} // namespace U2