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
    
private:
    MAlignmentObject*   maObj;
    int                 lastSavedObjectVersion;
    int                 maxMemUse;//in bytes;

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
