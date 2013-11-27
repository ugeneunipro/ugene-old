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

#ifndef _U2_MSA_EDITOR_SEQUENCE_AREA_H_
#define _U2_MSA_EDITOR_SEQUENCE_AREA_H_

#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QRubberBand>

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Core/Task.h>
#include <U2Gui/GScrollBar.h>


#include "DeleteGapsDialog.h"
#include "MSACollapsibleModel.h"
#include "MsaEditorUserModStepController.h"
#include "SaveSelectedSequenceFromMSADialogController.h"
#include "ExportHighlightedDialogController.h"

namespace U2 {

class MSAEditor;
class MSAEditorUI;
class GObjectView;
class MSAColorScheme;
class MSAHighlightingScheme;
class MAlignment;
class MAlignmentModInfo;
class MAlignmentObject;

class ModificationType {
public:
    static const int NoType = 0;
    static const int Reverse = 1;
    static const int Complement = 3;
    static const int ReverseComplement = 7;

    ModificationType(int _type = NoType) : type(_type) {}

    ModificationType operator + (const ModificationType & another) {
        switch (type + another.type) {
        case NoType + NoType: return ModificationType(NoType);
        case NoType + Reverse: return ModificationType(Reverse);
        case NoType + Complement: return ModificationType(Complement);
        case NoType + ReverseComplement: return ModificationType(ReverseComplement);
        case Reverse + Reverse: return ModificationType(NoType);
        case Reverse + Complement: return ModificationType(ReverseComplement);
        case Reverse + ReverseComplement: return ModificationType(Complement);
        case Complement + Complement: return ModificationType(NoType);
        case Complement + ReverseComplement: return ModificationType(Reverse);
        case ReverseComplement + ReverseComplement: return ModificationType(NoType);
        }
        return ModificationType(NoType);
    }

    int getType() {
        return type;
    }

    bool operator == (const ModificationType & another) {
        return type == another.type;
    }

    bool operator == (int _type) {
        return type == _type;
    }

    bool operator != (const ModificationType & another) {
        return type != another.type;
    }

    bool operator != (int _type) {
        return type != _type;
    }

    ModificationType & operator = (int _type) {
        type = _type;
        return *this;
    }

private:
    int type;
};

class U2VIEW_EXPORT MSAEditorSelection {
public:
    MSAEditorSelection() { }
    MSAEditorSelection(int left, int top, int width, int height) : selArea(left,top,width,height) { } 
    MSAEditorSelection(const QPoint& topLeft, const QPoint& bottomRight) : selArea(topLeft, bottomRight) { }
    MSAEditorSelection(const QPoint& topLeft, int width, int height) : selArea(topLeft, QSize(width,height)) { }
    
    // consider that selection may consist of several unconnected areas
    bool isContiniuous() const { return true; }

    bool isNull() const {return selArea.isNull(); }
    
    QPoint topLeft() const { return selArea.topLeft(); }
    
    const QRect& getRect() const {return selArea; }

    int x() const { return selArea.x(); }
    
    int y() const { return selArea.y(); }
    
    int width() const { return selArea.width(); }
    
    int height() const { return selArea.height(); }

    bool operator==(const MSAEditorSelection& other) const {
        return selArea == other.selArea;
    }
    
    MSAEditorSelection translated (int dx, int dy) const; 

    MSAEditorSelection intersected (const MSAEditorSelection& selection) const {
        QRect r = selArea.intersected(selection.selArea);
        return MSAEditorSelection(r);
    }

private:
    explicit MSAEditorSelection(QRect& rect) : selArea(rect) { } 
    QRect selArea;
};

class U2VIEW_EXPORT MSAEditorSequenceArea : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MSAEditorSequenceArea)
public:
    MSAEditorSequenceArea(MSAEditorUI* ui, GScrollBar* hb, GScrollBar* vb);
    ~MSAEditorSequenceArea();

    // x dimension -> positions
    int countWidthForBases(bool countClipped, bool forOffset = false) const;
    
    int getFirstVisibleBase() const {return startPos;}

    int getLastVisibleBase(bool countClipped, bool forOffset = false) const;

    int getNumVisibleBases(bool countClipped, bool forOffset = false) const; 

    U2Region getBaseXRange(int pos, bool useVirtualCoords) const;
    
    int getColumnNumByX(int x, bool selecting = false) const;
    int getXByColumnNum(int columnNum) const;

    void setFirstVisibleBase(int pos);


    // y dimension -> sequences
    int countHeightForSequences(bool countClipped) const;

    int getFirstVisibleSequence() const {return startSeq;} 
    
    int getLastVisibleSequence(bool countClipped) const;
    /*
     * Returns count of sequences that are visible on a screen.
     * @countClipped specifies whether include to result count or not last partially displayed row.
     */
    int getNumVisibleSequences(bool countClipped) const;
    /*
     * Returns count of sequences that are drawn on the widget by taking into account
     * collapsed rows.
     */
    int getNumDisplayedSequences( ) const;

    U2Region getSequenceYRange(int seqNum, bool useVirtualCoords) const;

    int getSequenceNumByY(int y) const;
    int getYBySequenceNum(int sequenceNum) const;

    void setFirstVisibleSequence(int seq);

    bool isAlignmentEmpty() const { return editor->isAlignmentEmpty(); }

    bool isPosInRange(int p) const;
    
    bool isSeqInRange(int s) const;
    
    bool isInRange(const QPoint& p) const {return isPosInRange(p.x()) && isSeqInRange(p.y());}

    bool isVisible(const QPoint& p, bool countClipped) const {return isPosVisible(p.x(), countClipped) && isSeqVisible(p.y(), countClipped);}
    
    bool isPosVisible(int pos, bool countClipped) const;
    
    bool isSeqVisible(int seq, bool countClipped) const;

    int coordToPos(int x) const;

    // returns valid position only for visible area
    QPoint coordToPos(const QPoint& coord) const;

    // returns valid position if coords are out of visible area
    QPoint coordToAbsolutePos(const QPoint& coord) const;
    QPoint coordToAbsolutePosOutOfRange(const QPoint& coord) const;

    const MSAEditorSelection& getSelection() const { assert(checkState()); return selection; }

    void updateSelection(const QPoint& newMousePos);

    // update selection when collapsible model changed
    void updateSelection();

    void setSelection(const MSAEditorSelection& sel);

    void moveSelection(int dx, int dy);

    /**
     * Shifts currently selected region to @shift.
     * If @shift > 0, the region is moved to the right and "true" is returned.
     * If @shift <= 0, the region is moved to the left only for the available number
     * of columns (i.e. the columns with gaps). The returned value specifies
     * whether the region was actually moved in this case.
     */
    bool shiftSelectedRegion(int shift);

    void cancelSelection();

    void setSelectionHighlighting( bool highlight = true );

    void deleteCurrentSelection();

    void addRowToSelection(int rowNumber) {selectedRows.append(rowNumber);}
    void deleteRowFromSelection(int rowNumber) {selectedRows.removeAll(rowNumber);}
    void clearSelection() {selectedRows.clear();}

    U2Region getSelectedRows() const;

    U2Region getRowsAt(int seq) const;

    QPair<QString, int> getGappedColumnInfo() const;

    int getHeight();

    QStringList getAvailableColorSchemes() const;

    QStringList getAvailableHighlightingSchemes() const;

    bool hasAminoAlphabet();
private:
    // emulating cursor mode with

    void setCursorPos(const QPoint& p);

    void setCursorPos(int x, int y) { setCursorPos(QPoint(x, y)); }
    
    void setCursorPos(int pos) { setCursorPos(QPoint(pos, cursorPos.y())); }

    void moveCursor(int dx, int dy);

    void highlightCurrentCursorPos()  { highlightSelection = true; update();}

    
public:    
    void centerPos(const QPoint& pos);
    
    void centerPos(int pos);

    void setFont(const QFont& f);

    GScrollBar* getVBar() const {return svBar;}

    GScrollBar* getHBar() const {return shBar;}

    void updateHBarPosition(int base);

    void updateVBarPosition(int seq);
    
    void drawContent(QPainter& p);

    QString exportHighligtning(int startPos, int endPos, int startingIndex, bool keepGaps, bool dots);

    MSAColorScheme* getCurrentColorScheme(){return colorScheme;}
    MSAHighlightingScheme* getCurrentHighlightingScheme(){return highlitingScheme;}
    bool getUseDotsCheckedState(){return useDotsAction->isChecked();}

    void onVisibleRangeChanged();
signals:
    void si_startChanged(const QPoint& p, const QPoint& prev);
    void si_selectionChanged(const MSAEditorSelection& current, const MSAEditorSelection& prev);
    void si_selectionChanged(const QStringList& selectedRows);
    void si_highlightingChanged();
    void si_visibleRangeChanged(QStringList visibleSequences, int reqHeight);

public slots:
    void sl_changeColorSchemeOutside(const QString &name);
    void sl_doUseDots();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent*); 
    void mouseMoveEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent *);
    void focusOutEvent(QFocusEvent* fe); 
    void focusInEvent(QFocusEvent* fe);

private slots:
    void sl_onHScrollMoved(int pos);
    void sl_onVScrollMoved(int pos);
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&);

    void sl_buildStaticMenu(GObjectView* v, QMenu* m);
    void sl_buildStaticToolbar(GObjectView* v, QToolBar* t);
    void sl_buildContextMenu(GObjectView* v, QMenu* m);
    void sl_lockedStateChanged();
    void sl_addSeqFromFile();
    void sl_addSeqFromProject();
    
    void sl_delCurrentSelection();
    void sl_copyCurrentSelection();
    void sl_fillCurrentSelectionWithGaps();
    void sl_delCol();
    void sl_goto();
    void sl_removeAllGaps();
    void sl_sortByName();
    void sl_setCollapsingMode(bool enabled);
    void sl_updateCollapsingMode();
    void sl_reverseComplementCurrentSelection();
    void sl_reverseCurrentSelection();
    void sl_complementCurrentSelection();

    void sl_onPosChangeRequest(int pos);

    void sl_createSubaligniment();

    void sl_saveSequence();

    void sl_changeColorScheme();
    void sl_changeHighlightScheme();

    void sl_zoomOperationPerformed(bool resizeModeChanged);

    void sl_modelChanged();

    void sl_customColorSettingsChanged();
    void sl_showCustomSettings();
    void sl_referenceSeqChanged(qint64);

    void sl_resetCollapsibleModel();
    void sl_setCollapsingRegions(const QList<QStringList>&);
    void sl_useDots();
    void sl_fontChanged(QFont font);

protected:
    virtual void wheelEvent (QWheelEvent * event);

private:
    void buildMenu(QMenu* m);
    void prepareColorSchemeMenuActions();
    void prepareHighlightingMenuActions();

    void initCustomSchemeActions(const QString& id, DNAAlphabetType type);

    void updateActions();
    
    void updateHScrollBar();
    void updateVScrollBar();
    
    void drawAll();
    void drawCursor(QPainter& p);
    void drawFocus(QPainter& p);
    void drawSelection(QPainter &p);

    /**
     * Inserts a region consisting of gaps only before the selection. The inserted region width
     * is specified by @countOfGaps parameter if 0 < @countOfGaps, its height is equal to the
     * current selection's height.
     *
     * If there is no selection in MSA then the method does nothing.
     *
     * If -1 == @countOfGaps then the inserting region width is equal to
     * the selection's width. If 1 > @countOfGaps and -1 != @countOfGaps then nothing happens.
     */
    void insertGapsBeforeSelection( int countOfGaps = -1 );

    /**
     * Reverse operation for @insertGapsBeforeSelection( ),
     * removes the region preceding the selection if it consists of gaps only.
     *
     * If there is no selection in MSA then the method does nothing.
     *
     * @countOfGaps specifies maximum width of the removed region.
     * If -1 == @countOfGaps then count of removed gap columns is equal to
     * the selection width. If 1 > @countOfGaps and -1 != @countOfGaps then nothing happens.
     */
    void removeGapsPrecedingSelection( int countOfGaps = -1 );
  
    void deleteOldCustomSchemes();

    bool checkState() const;
    void validateRanges();          //called on resize/refont like events

    void reverseComplementModification(ModificationType& type);

    /*
     * Interrupts the tracking of MSA modifications caused by a region shifting,
     * also stops shifting. The method is used to keep consistence of undo/redo stack.
     */
    void cancelShiftTracking( );

    MSAEditor*      editor;
    MSAEditorUI*    ui;
    GScrollBar*     shBar;
    GScrollBar*     svBar;
    QRubberBand*    rubberBand;

    int             startPos; //first visible x pos 
    int             startSeq; //first visible y pos

    bool                shifting;
    bool                selecting;
    bool                shiftingWasPerformed; // specifies whether a user has shifted a selection
    QPoint              origin; // global window coordinates
    QPoint              cursorPos; // mouse cursor position in alignment coordinates
    MSAEditorSelection  selection; // selection with rows indexes in collapsible model coordinates
    MSAEditorSelection  baseSelection; // selection with rows indexes in absolute coordinates
    QList<int>          selectedRows;
    QStringList         selectedRowNames;

    QAction*        copySelectionAction;
    QAction*        delSelectionAction;
    QAction*        delColAction;
    QAction*        insSymAction;
    QAction*        removeAllGapsAction;
    QAction*        gotoAction;
    QAction*        createSubaligniment;
    QAction*        saveSequence;
    QAction*        addSeqFromFileAction;
    QAction*        addSeqFromProjectAction;
    QAction*        sortByNameAction;
    QAction*        collapseModeSwitchAction;
    QAction*        collapseModeUpdateAction;
    QAction*        reverseComplementAction;
    QAction*        reverseAction;
    QAction*        complementAction;
    QAction*        lookMSASchemesSettingsAction;
    QAction*        useDotsAction;
    
    QPixmap*        cachedView;
    bool            completeRedraw;

    MSAColorScheme* colorScheme;
    MSAHighlightingScheme* highlitingScheme;
    bool            highlightSelection;

    QList<QAction*> colorSchemeMenuActions;
    QList<QAction* > customColorSchemeMenuActions;
    QList<QAction* > highlightingSchemeMenuActions;

    // The member is intended for tracking MSA changes (handling U2UseCommonUserModStep objects)
    // that does not fit into one method, e.g. shifting MSA region with mouse.
    // If the changing action fits within one method it's recommended using
    // the U2UseCommonUserModStep object explicitly.
    MsaEditorUserModStepController changeTracker;
};

class U2VIEW_EXPORT ExportHighligtningTask : public Task {
    Q_OBJECT
public:
    ExportHighligtningTask(ExportHighligtningDialogController *dialog, MSAEditorSequenceArea *msaese_);

    void run();

private:
    int startPos;
    int endPos;
    int startingIndex;
    bool keepGaps;
    bool dots;
    GUrl url;
    MSAEditorSequenceArea *msaese;
};

}//namespace
#endif
