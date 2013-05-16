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

#include "UndoRedoFramework.h"
#include "ov_msa/MSACollapsibleModel.h"

#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>


namespace U2 {

MsaUndoRedoFramework::MsaUndoRedoFramework(QObject *p, MAlignmentObject *_maObj)
: QObject(p),
  maObj(_maObj),
  undoStepsAvailable(0),
  redoStepsAvailable(0)
{
    SAFE_POINT(maObj != NULL, "NULL MSA Object!", );

    undoAction = new QAction(this);
    undoAction->setObjectName("Undo");
    undoAction->setIcon(QIcon(":core/images/undo.png"));
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setToolTip(QString("%1 (%2)").arg(undoAction->text()).arg(undoAction->shortcut().toString()));

    redoAction = new QAction(this);
    redoAction->setObjectName("Redo");
    redoAction->setIcon(QIcon(":core/images/redo.png"));
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setToolTip(QString("%1 (%2)").arg(redoAction->text()).arg(redoAction->shortcut().toString()));

    checkUndoRedoEnabled();

    connect(maObj, SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)), 
                   SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
    connect(maObj, SIGNAL(si_completeStateChanged(bool)), SLOT(sl_completeStateChanged(bool)));
    connect(maObj, SIGNAL(si_lockedStateChanged()), SLOT(sl_lockedStateChanged()));
    connect(undoAction, SIGNAL(triggered()), this, SLOT(sl_undo()));
    connect(redoAction, SIGNAL(triggered()), this, SLOT(sl_redo()));
    stateComplete = true;
}

void MsaUndoRedoFramework::sl_completeStateChanged(bool _stateComplete) {
    stateComplete = _stateComplete;
}

void MsaUndoRedoFramework::sl_lockedStateChanged() {
    checkUndoRedoEnabled();
}

void MsaUndoRedoFramework::sl_alignmentChanged(const MAlignment& /* maBefore*/ , const MAlignmentModInfo& modInfo) {
    checkUndoRedoEnabled();
}

void MsaUndoRedoFramework::checkUndoRedoEnabled() {
    SAFE_POINT(maObj != NULL, "NULL MSA Object!", );

    bool b = maObj->isStateLocked();
    if (maObj->isStateLocked() || !stateComplete) {
        undoAction->setEnabled(false);
        redoAction->setEnabled(false);
        return;
    }

    U2OpStatus2Log os;
    DbiConnection con(maObj->getEntityRef().dbiRef, os);
    SAFE_POINT_OP(os, );

    U2ObjectDbi* objDbi = con.dbi->getObjectDbi();
    SAFE_POINT(NULL != objDbi, "NULL Object Dbi!", );

    bool enableUndo = objDbi->canUndo(maObj->getEntityRef().entityId, os);
    SAFE_POINT_OP(os, );
    bool enableRedo = objDbi->canRedo(maObj->getEntityRef().entityId, os);
    SAFE_POINT_OP(os, );

    undoAction->setEnabled(enableUndo);
    redoAction->setEnabled(enableRedo);
}

void MsaUndoRedoFramework::sl_undo() {
    SAFE_POINT(maObj != NULL, "NULL MSA Object!", );

    U2OpStatus2Log os;
    U2EntityRef msaRef =  maObj->getEntityRef();

    assert(stateComplete);
    assert(!maObj->isStateLocked());

    DbiConnection con(msaRef.dbiRef, os);
    SAFE_POINT_OP(os, );

    U2ObjectDbi* objDbi = con.dbi->getObjectDbi();
    SAFE_POINT(NULL != objDbi, "NULL Object Dbi!", );

    objDbi->undo(msaRef.entityId, os);
    SAFE_POINT_OP(os, );

    maObj->updateCachedMAlignment();
}

void MsaUndoRedoFramework::sl_redo() {
    SAFE_POINT(maObj != NULL, "NULL MSA Object!", );

    U2OpStatus2Log os; 
    U2EntityRef msaRef =  maObj->getEntityRef();

    assert(stateComplete);
    assert(!maObj->isStateLocked());

    DbiConnection con(msaRef.dbiRef, os);
    SAFE_POINT_OP(os, );

    U2ObjectDbi* objDbi = con.dbi->getObjectDbi();
    SAFE_POINT(NULL != objDbi, "NULL Object Dbi!", );

    objDbi->redo(msaRef.entityId, os);
    SAFE_POINT_OP(os, );

    maObj->updateCachedMAlignment();
}


} // namespace
