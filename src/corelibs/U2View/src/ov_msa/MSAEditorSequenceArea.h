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

#include "DeleteGapsDialog.h"
#include "MSACollapsibleModel.h"
#include "SaveSelectedSequenceFromMSADialogController.h"

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Gui/GScrollBar.h>

#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QRubberBand>

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

    int getNumVisibleSequences(bool countClipped) const;

    U2Region getSequenceYRange(int seqNum, bool useVirtualCoords) const;

    int getSequenceNumByY(int y) const;
    int getYBySequenceNum(int sequenceNum) const;

    void setFirstVisibleSequence(int seq);

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

    const MSAEditorSelection& getSelection() const { assert(checkState()); return selection; }

    void updateSelection(const QPoint& newMousePos);

    void setSelection(const MSAEditorSelection& sel);

    void moveSelection(int dx, int dy);

    void shiftSelectedRegion(int shift);

    void cancelSelection();

    void highlightCurrentSelection() { highlightSelection = true; update(); }

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

private:
    // emulating cursor mode with

    void setCursorPos(const QPoint& p);

    void setCursorPos(int x, int y) { setCursorPos(QPoint(x, y)); };
    
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

    MSAColorScheme* getCurrentColorScheme(){return colorScheme;};
    MSAHighlightingScheme* getCurrentHighlightingScheme(){return highlitingScheme;};
    bool getUseDotsCheckedState(){return useDotsAction->isChecked();};

signals:
    void si_startChanged(const QPoint& p, const QPoint& prev);
    void si_selectionChanged(const MSAEditorSelection& current, const MSAEditorSelection& prev);
    void si_selectionChanged(const QStringList& selectedRows);
    void si_highlightingChanged();
    void si_visibleRangeChanged(QStringList visibleSequences);

public slots:
    void sl_changeColorSchemeOutside(const QString &name);
    void sl_useDots(int);

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
    void sl_onScrollBarActionTriggered(int scrollAction);

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
    void sl_setSeqAsRefrence();

    void sl_onPosChangeRequest(int pos);

    void sl_createSubaligniment();

    void sl_saveSequence();

    void sl_changeColorScheme();
    void sl_changeHighlightScheme();

    void sl_zoomOperationPerformed(bool resizeModeChanged);

    void sl_modelChanged();

    void sl_customColorSettingsChanged();
    void sl_showCustomSettings();
    void sl_refrenceSeqChanged(const QString &str);

    void sl_resetCollapsibleModel();
    void sl_setCollapsingRegions(const QStringList* visibleSequences);
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

    void fillSelectionWithGaps();
    void del(const QPoint& p, bool columnMode);
  
    void deleteOldCustomSchemes();

    bool checkState() const;
    void validateRanges();          //called on resize/refont like events

    void reverseComplementModification(ModificationType& type);

    MSAEditor*      editor;
    MSAEditorUI*    ui;
    GScrollBar*     shBar;
    GScrollBar*     svBar;
    QRubberBand*    rubberBand;

    int             startPos; //first visible x pos 
    int             startSeq; //first visible y pos

    bool                scribbling, shifting, selecting;
    QPoint              origin; // global window coordinates
    QPoint              cursorPos; // mouse cursor position in alignment coordinates
    MSAEditorSelection  selection;
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

    QMutex lock;
};

}//namespace
#endif
