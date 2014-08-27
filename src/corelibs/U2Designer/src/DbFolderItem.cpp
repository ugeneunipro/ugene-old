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

#include <U2Core/Folder.h>
#include <U2Core/U2ObjectDbi.h>

#include <U2Lang/SharedDbUrlUtils.h>

#include "DbFolderItem.h"

namespace U2 {

DbFolderItem::DbFolderItem(const QString &url, QListWidget *parent)
    : DirectoryItem(url, parent)
{
    QIcon dirIcon = QIcon(QString(":U2Designer/images/database_folder.png"));
    setIcon(dirIcon);

    const QString folderPath = SharedDbUrlUtils::getDbFolderPathByUrl(url);
    setToolTip("<p><b>" + SharedDbUrlUtils::getDbShortNameFromEntityUrl(url) + "</b>: " + folderPath + "</p><p>" + tr("Use <i>right click</i> to set advanced options") + "</p>");

    QString folderName = Folder::getFolderName(folderPath);
    if (folderName.isEmpty()) {
        folderName = U2ObjectDbi::ROOT_FOLDER;
    }

    setText(folderName);
}

} // namespace U2
