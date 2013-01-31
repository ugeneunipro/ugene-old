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

#ifndef _U2_MSA_EDITOR_UNDO_FRAMEWOFK_H_
#define _U2_MSA_EDITOR_UNDO_FRAMEWOFK_H_

#include <U2Core/global.h>
#include <U2Core/MAlignment.h>

#include <QtGui/QUndoStack>

namespace U2 {

class MAlignmentObject;
class MAlignmentModInfo;

class U2VIEW_EXPORT MSAEditorUndoFramework : protected QUndoStack {
    Q_OBJECT
public:
    MSAEditorUndoFramework(QObject* p, MAlignmentObject* ma);
    void applyUndoRedoAction(const MAlignment& ma);

    QAction* getUndoAction() const {return uAction;}
    QAction* getRedoAction() const {return rAction;}

public slots:
    void sl_alignmentChanged(const MAlignment& aliBefore, const MAlignmentModInfo& modInfo);
    void sl_lockedStateChanged();
    void sl_completeStateChanged(bool complete);
    
private:
    MAlignmentObject*   maObj;
    int                 lastSavedObjectVersion;
    int                 maxMemUse;//in bytes;
    bool                stateComplete;

    QAction*            uAction;
    QAction*            rAction;

};

class MSAEditorUndoCommand : public QUndoCommand {
    friend class MSAEditorUndoFramework;
public:
    MSAEditorUndoCommand() : fwk (NULL) {}
    virtual int getMemUsage() const {return 0;} //in bytes, report non-0 if > 100k;

protected:
    MSAEditorUndoFramework* fwk;
};

class MSAEditorUndoWholeAliCommand : public MSAEditorUndoCommand {
public:
    MSAEditorUndoWholeAliCommand(const MAlignment& _maBefore, const MAlignment& _maAfter) : maBefore(_maBefore), maAfter(_maAfter){}
    
    virtual int getMemUsage() const {return maBefore.estimateMemorySize();}
    
    virtual void redo();
    virtual void undo();

protected:
    MAlignment maBefore;
    MAlignment maAfter;
};


} // namespace

#endif
