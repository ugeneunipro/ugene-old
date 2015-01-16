/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "WorkflowMetadata.h"

namespace U2 {

WorkflowMetadata::WorkflowMetadata()
: id(-1)
{

}

WorkflowMetadata::WorkflowMetadata(const QString &datasetName)
: id(nextid()), datasetName(datasetName)
{

}

WorkflowMetadata::WorkflowMetadata(const QString &fileUrl, const QString &datasetName)
: id(nextid()), datasetName(datasetName), fileUrl(fileUrl)
{

}

WorkflowMetadata::WorkflowMetadata(const QString &databaseUrl, const QString &databaseId, const QString &datasetName)
: id(nextid()), datasetName(datasetName), databaseUrl(databaseUrl), databaseId(databaseId)
{

}

int WorkflowMetadata::getId() const {
    return id;
}

const QString & WorkflowMetadata::getDatasetName() const {
    return datasetName;
}

const QString & WorkflowMetadata::getFileUrl() const {
    return fileUrl;
}

const QString & WorkflowMetadata::getDatabaseUrl() const {
    return databaseUrl;
}

const QString & WorkflowMetadata::getDatabaseId() const {
    return databaseId;
}

int WorkflowMetadata::nextid() {
    static QAtomicInt id(0);
    return id.fetchAndAddRelaxed(1);
}

/************************************************************************/
/* WorkflowMetadataStorage */
/************************************************************************/
void WorkflowMetadataStorage::put(const WorkflowMetadata &value) {
    data[value.getId()] = value;
}

WorkflowMetadata WorkflowMetadataStorage::get(int metadataId) const {
    return data.value(metadataId, WorkflowMetadata());
}

} // U2
