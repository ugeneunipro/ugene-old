/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <U2View/MSAEditor.h>

#include "AlignMsaAction.h"

namespace U2 {

MSAEditor* AlignMsaAction::getMsaEditor() const {
    MSAEditor* e = qobject_cast<MSAEditor*>(getObjectView());
    SAFE_POINT(e != NULL, "Can't get an appropriate MSA Editor", NULL);
    return e;
}

void AlignMsaAction::sl_updateState() {
    StateLockableItem* item = qobject_cast<StateLockableItem*>(sender());
    SAFE_POINT(item != NULL, "Unexpected sender: expect StateLockableItem", );
    MSAEditor* msaEditor = getMsaEditor();
    CHECK(msaEditor != NULL, );
    setEnabled(!item->isStateLocked() && !msaEditor->isAlignmentEmpty());
}

}   // namespace U2
