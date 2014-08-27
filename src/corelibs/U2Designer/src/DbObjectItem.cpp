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

#include <U2Core/GObjectTypes.h>

#include <U2Lang/SharedDbUrlUtils.h>

#include "DbObjectItem.h"

namespace U2 {

DbObjectItem::DbObjectItem(const QString &url, QListWidget *parent)
    : FileItem(url, parent)
{
    QIcon fileIcon = GObjectTypes::getTypeInfo(SharedDbUrlUtils::getDbObjectTypeByUrl(url)).icon;
    setIcon(fileIcon);

    const QString objectName = SharedDbUrlUtils::getDbObjectNameByUrl(url);
    setText(objectName);
    setToolTip("\"" + objectName + tr("\" is located in the database <b>") + SharedDbUrlUtils::getDbShortNameFromEntityUrl(url) + "</b>");
}

} // U2
