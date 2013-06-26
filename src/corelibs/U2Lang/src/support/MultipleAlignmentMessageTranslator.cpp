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

#include <U2Core/MAlignmentObject.h>
#include <U2Lang/WorkflowContext.h>
#include <U2Lang/DbiDataHandler.h>

#include "MultipleAlignmentMessageTranslator.h"

const char *ALIGNMENT_NAME_LABEL = "Name: ";
const char *EMPTY_ALIGNMENT_NAME_LABEL = "(empty)";
const char *ALIGNMENT_LENGTH_LABEL = " Length: ";
const char *COUNT_OF_ROWS_LABEL = " Count of rows: ";
const char *ROW_NAMES_LABEL = " Row names: ";

namespace U2 {

MultipleAlignmentMessageTranslator::MultipleAlignmentMessageTranslator(
    const QVariant &atomicMessage, WorkflowContext *initContext)
    : BaseMessageTranslator(atomicMessage, initContext)
{
    Q_ASSERT(source.canConvert<SharedDbiDataHandler>());
    SharedDbiDataHandler malignmentId = source.value<SharedDbiDataHandler>();
    std::auto_ptr<MAlignmentObject> malignmentObject(StorageUtils::getMsaObject(
        context->getDataStorage(), malignmentId));
    Q_ASSERT(NULL != malignmentObject.get());
    malignment = malignmentObject->getMAlignment();
}

MultipleAlignmentMessageTranslator::~MultipleAlignmentMessageTranslator() {

}

QString MultipleAlignmentMessageTranslator::getTranslation() const {
    const QString alignmentName = malignment.getName();
    const QString displayingName = (alignmentName.isEmpty())
        ? QObject::tr(EMPTY_ALIGNMENT_NAME_LABEL) : ("'" + alignmentName + "'");

    QString result = QString().append(QObject::tr(ALIGNMENT_NAME_LABEL)
        + displayingName + INFO_TAGS_SEPARATOR);
    result.append(QObject::tr(ALIGNMENT_LENGTH_LABEL) + QString::number(malignment.getLength())
        + INFO_TAGS_SEPARATOR);
    result.append(QObject::tr(COUNT_OF_ROWS_LABEL) + QString::number(malignment.getNumRows())
        + INFO_TAGS_SEPARATOR);
    result.append(QObject::tr(ROW_NAMES_LABEL) + "'" + malignment.getRowNames().join("', '")
        + "'");

    return result;
}

} // namespace U2