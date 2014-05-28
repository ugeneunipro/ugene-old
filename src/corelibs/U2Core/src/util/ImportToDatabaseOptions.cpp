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
    keepFoldersStructure(true),
    processFoldersRecursively(true),
    createSubfolderForTopLevelFolder(false),
    createSubfolderForEachFile(true),
    importUnknownAsUdr(false),
    multiSequencePolicy(SEPARATE),
    createSubfolderForEachDocument(true),
    mergeMultiSequencePolicySeparatorSize(10)
{
}

bool ImportToDatabaseOptions::operator == (const ImportToDatabaseOptions &other) const {
    return keepFoldersStructure == other.keepFoldersStructure &&
            processFoldersRecursively == other.processFoldersRecursively &&
            createSubfolderForTopLevelFolder == other.createSubfolderForTopLevelFolder &&
            createSubfolderForEachFile == other.createSubfolderForEachFile &&
            importUnknownAsUdr == other.importUnknownAsUdr &&
            multiSequencePolicy == other.multiSequencePolicy &&
            createSubfolderForEachDocument == other.createSubfolderForEachDocument &&
            mergeMultiSequencePolicySeparatorSize == other.mergeMultiSequencePolicySeparatorSize;
}

bool ImportToDatabaseOptions::operator !=(const ImportToDatabaseOptions &other) const {
    return !operator ==(other);
}

}   // namespace U2
