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

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Lang/WorkflowContext.h>
#include <U2Lang/DbiDataHandler.h>

#include "AssemblyMessageTranslator.h"

const char *ASSEMBLY_LENGTH_LABEL = "Length: ";
const char *COUNT_OF_READS_LABEL = " Count of reads: ";

namespace U2 {

AssemblyMessageTranslator::AssemblyMessageTranslator(const QVariant &atomicMessage, WorkflowContext *initContext)
    : BaseMessageTranslator(atomicMessage, initContext)
{
    SAFE_POINT(source.canConvert<SharedDbiDataHandler>(), "Message doesn't contain dbi reference", );
    SharedDbiDataHandler dbId = source.value<SharedDbiDataHandler>();
    std::auto_ptr<AssemblyObject> assemblyObject(StorageUtils::getAssemblyObject(context->getDataStorage(), dbId));
    SAFE_POINT(NULL != assemblyObject.get(), "Couldn't obtain assembly object", );
    assemblyRef = assemblyObject->getEntityRef();
}

AssemblyMessageTranslator::~AssemblyMessageTranslator() {

}

QString AssemblyMessageTranslator::getTranslation() const {
    U2OpStatusImpl os;
    DbiConnection connection(assemblyRef.dbiRef, os);
    SAFE_POINT_OP(os, "Couldn't establish dbi connection");

    U2AssemblyDbi *dbi = connection.dbi->getAssemblyDbi();
    const U2DataId assemblyId = assemblyRef.entityId;
    const qint64 assemblyLength = dbi->getMaxEndPos(assemblyId, os) + 1;
    SAFE_POINT_OP(os, "Couldn't obtain assembly length");

    const U2Region wholeAssembly(0, assemblyLength);
    const qint64 countOfReads = dbi->countReads(assemblyId, wholeAssembly, os);
    SAFE_POINT_OP(os, "Couldn't obtain count of reads in assembly");

    QString result = QString().append(QObject::tr(ASSEMBLY_LENGTH_LABEL)
        + QString::number(assemblyLength) + INFO_FEATURES_SEPARATOR);
    result.append(QObject::tr(COUNT_OF_READS_LABEL) + QString::number(countOfReads));

    return result;
}

} // namespace U2
