/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "U2ObjectDbi.h"

namespace U2 {

const QString U2ObjectDbi::ROOT_FOLDER                  = "/";
const QString U2ObjectDbi::RECYCLE_BIN_FOLDER           = "Recycle bin";
const QString U2ObjectDbi::PATH_SEP                     = "/";

const int U2ObjectDbi::OBJECT_ACCESS_UPDATE_INTERVAL    = 5000;

const QString U2ObjectDbi::PREV_OBJ_PATH_ATTR_NAME      = "Previous path";

void U2ObjectDbi::updateObjectAccessTime(const U2DataId &/*objectId*/, U2OpStatus &/*os*/) {
}

bool U2ObjectDbi::isObjectInUse(const U2DataId &/*id*/, U2OpStatus &/*os*/) {
    return false;
}

QList<U2DataId> U2ObjectDbi::getAllObjectsInUse(U2OpStatus &/*os*/) {
    return QList<U2DataId>();
}

} // namespace U2
