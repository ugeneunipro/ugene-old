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

#include <QtCore/QFileInfo>

#include <U2Core/U2SafePoints.h>

#include "ImportToDatabaseOptions.h"

namespace U2 {

ImportToDatabaseOptions::ImportToDatabaseOptions() :
    createSubfolderForEachDocument(true),
    createSubfolderForEachFile(true),
    createSubfolderForTopLevelFolder(false),
    importUnknownAsUdr(false),
    keepFileExtension(false),
    keepFoldersStructure(true),
    mergeMultiSequencePolicySeparatorSize(10),
    multiSequencePolicy(SEPARATE),
    processFoldersRecursively(true)
{
}

bool ImportToDatabaseOptions::operator == (const ImportToDatabaseOptions &other) const {
    return  createSubfolderForEachDocument == other.createSubfolderForEachDocument &&
            createSubfolderForEachFile == other.createSubfolderForEachFile &&
            createSubfolderForTopLevelFolder == other.createSubfolderForTopLevelFolder &&
            importUnknownAsUdr == other.importUnknownAsUdr &&
            keepFileExtension == other.keepFileExtension &&
            keepFoldersStructure == other.keepFoldersStructure &&
            mergeMultiSequencePolicySeparatorSize == other.mergeMultiSequencePolicySeparatorSize &&
            multiSequencePolicy == other.multiSequencePolicy &&
            processFoldersRecursively == other.processFoldersRecursively;
}

bool ImportToDatabaseOptions::operator !=(const ImportToDatabaseOptions &other) const {
    return !operator ==(other);
}

}   // namespace U2
