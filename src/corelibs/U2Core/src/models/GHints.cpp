/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "GHints.h"

#include "StateLockableDataModel.h"

namespace U2 {

void ModTrackHints::setMap(const QVariantMap& _map) {
    if (map == _map) {
        return;
    }
    map = _map;
    setModified();
}

void ModTrackHints::set(const QString& key, const QVariant& val) {
    if (get(key) == val) {
        return;
    }
    map[key] = val;
    setModified();
}

int ModTrackHints::remove(const QString& key) {
    int r = map.remove(key);
    if (r!=0) {
        setModified();
    }
    return r;
}

void ModTrackHints::setModified() {
    StateLockableTreeItem* modItem = p;
    while (modItem->getParentStateLockItem()!=NULL && topParentMode) {
        modItem = modItem->getParentStateLockItem();
    }
    if (!modItem->isStateLocked()) { //TODO:
        modItem->setModified(true);
    }
}

}//namespace

