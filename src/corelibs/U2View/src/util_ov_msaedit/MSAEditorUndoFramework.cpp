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

#include "MSAEditorUndoFramework.h"

#include <U2Core/MAlignmentObject.h>

#include <QtGui/QAction>

namespace U2 {

MSAEditorUndoFramework::MSAEditorUndoFramework(QObject* p, MAlignmentObject* ma) 
: QUndoStack(p), maObj(ma), lastSavedObjectVersion(0), maxMemUse(20*1024*1024)
{
    if (maObj!=NULL) {
        connect(maObj, SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)), 
                       SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
        connect(maObj, SIGNAL(si_lockedStateChanged()), SLOT(sl_lockedStateChanged()));
    }
    
    setUndoLimit(100);
    
    uAction = createUndoAction(this);
    uAction->setIcon(QIcon(":core/images/undo.png"));
    uAction->setShortcut(QKeySequence::Undo);
    uAction->setToolTip(QString("%1 (%2)").arg(uAction->text()).arg(uAction->shortcut().toString()));

    rAction = createRedoAction(this);
    rAction->setIcon(QIcon(":core/images/redo.png"));
    rAction->setShortcut(QKeySequence::Redo);
    rAction->setToolTip(QString("%1 (%2)").arg(rAction->text()).arg(rAction->shortcut().toString()));


    sl_lockedStateChanged();//updates action state
}

void MSAEditorUndoFramework::sl_lockedStateChanged() {
    bool active = maObj ? !maObj->isStateLocked() : false;
    setActive(active);
    
    int activeIdx = index();
    int cnt = count();
    uAction->setEnabled(active && activeIdx > 0);
    rAction->setEnabled(active && activeIdx + 1 < cnt);
}

void MSAEditorUndoFramework::applyUndoRedoAction(const MAlignment& ma) {
    if (maObj) {
        assert(!maObj->isStateLocked());
        lastSavedObjectVersion = maObj->getModificationVersion()+1;
        maObj->setMAlignment(ma);
    }
}

void MSAEditorUndoFramework::sl_alignmentChanged(const MAlignment& maBefore, const MAlignmentModInfo& ) {
    if (maObj == NULL || lastSavedObjectVersion == maObj->getModificationVersion() || maBefore.getRows() == maObj->getMAlignment().getRows()) {
        return;
    }
    lastSavedObjectVersion = maObj->getModificationVersion();
    const MAlignment& maAfter = maObj->getMAlignment();
    int memUseBefore = 0;
    int cntBefore = count();
    for (int i=0; i < cntBefore;i++) {
        memUseBefore+=(static_cast<const MSAEditorUndoCommand*>(command(i)))->getMemUsage();
    }
    MSAEditorUndoWholeAliCommand* cmd = new MSAEditorUndoWholeAliCommand(maBefore, maAfter);
    cmd->fwk = this;
    int cmdMemUse = cmd->getMemUsage();
    int undoL = undoLimit();
    if (memUseBefore + cmdMemUse < maxMemUse && cntBefore == undoL) {
        setUndoLimit(undoL+1);
    }
    push(cmd);

    int newMemUse = memUseBefore + cmdMemUse;
    if (newMemUse > maxMemUse) {
        int itemsToRemove = 0;
        int dMem = 0;
        int cnt  = count();
        for (; itemsToRemove < cnt; itemsToRemove++) {
            dMem+=(static_cast<const MSAEditorUndoCommand*>(command(itemsToRemove)))->getMemUsage();    
            if (newMemUse - dMem <= maxMemUse) {
                break;
            }
        }
        int newLimit = cnt - itemsToRemove;
        setUndoLimit(newLimit);
    }
}

void MSAEditorUndoWholeAliCommand::redo() {
    fwk->applyUndoRedoAction(maAfter);
}

void MSAEditorUndoWholeAliCommand::undo() {
    fwk->applyUndoRedoAction(maBefore);
}

} //namespace
