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

#include <QtCore/QVariant>

#include <U2Lang/WorkflowContext.h>
#include <U2Lang/DbiDataHandler.h>

#include "SequenceMessageTranslator.h"

const char *SEQUENCE_NAME_LABEL = "Name: ";
const char *SEQUENCE_LENGTH_LABEL = " Length: ";
const char *SEQUENCE_CONTENT_LABEL = " Content: ";
const char *SEQUENCE_CONTENT_ENDING = "...";
const int COUNT_OF_DISPLAYING_SEQUENCE_SYMBOLS = 100;

namespace U2 {

SequenceMessageTranslator::SequenceMessageTranslator(const QVariant &atomicMessage, WorkflowContext *initContext)
    : BaseMessageTranslator(atomicMessage, initContext)
{
    Q_ASSERT(source.canConvert<SharedDbiDataHandler>());
    SharedDbiDataHandler sequenceId = source.value<SharedDbiDataHandler>();
    sequenceObject = StorageUtils::getSequenceObject(context->getDataStorage(), sequenceId);
    Q_ASSERT(NULL != sequenceObject);
}

SequenceMessageTranslator::~SequenceMessageTranslator() {

}

QString SequenceMessageTranslator::getTranslation() const {
    QString result = QString().append(QObject::tr(SEQUENCE_NAME_LABEL)
        + sequenceObject->getSequenceName() + INFO_TAGS_SEPARATOR);
    const int sequenceLength = sequenceObject->getSequenceLength();
    result.append(QObject::tr(SEQUENCE_LENGTH_LABEL) + QString::number(sequenceLength)
        + INFO_TAGS_SEPARATOR);
    result.append(QObject::tr(SEQUENCE_CONTENT_LABEL)
        + sequenceObject->getSequenceData(U2Region(0, COUNT_OF_DISPLAYING_SEQUENCE_SYMBOLS)));
    if(sequenceLength > COUNT_OF_DISPLAYING_SEQUENCE_SYMBOLS) {
        result.append(QObject::tr(SEQUENCE_CONTENT_ENDING));
    }
    
    return result;
}

} // namespace U2