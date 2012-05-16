/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_EDITOR_NAME_LIST_H_
#define _U2_MSA_EDITOR_NAME_LIST_H_

#include "MSACollapsibleModel.h"

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <QtGui/QMenu>
#include <QtGui/QScrollBar>
#include <QtGui/QRubberBand>

namespace U2 {

class MSAEditor;
class MSAEditorUI;
class GObjectView;
class MAlignment;
class MAlignmentModInfo;
class MSAEditorSelection;

class MSAEditorNameList: public QWidget {
    Q_OBJECT
public:
    MSAEditorNameList(MSAEditorUI* ui, QScrollBar* nhBar);
    ~MSAEditorNameList();

private slots:
    void sl_buildStaticMenu(GObjectView* v, QMenu* m);
    void sl_buildContextMenu(GObjectView* v, QMenu* m);
    void sl_copyCurrentSequence();
    void sl_editSequenceName();
    void sl_lockedStateChanged();
    void sl_removeCurrentSequence();
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&);
    void sl_onScrollBarActionTriggered( int scrollAction );
    
    void sl_startChanged(const QPoint& p, const QPoint& prev);
    void sl_selectionChanged(const MSAEditorSelection& current, const MSAEditorSelection& prev);

    void sl_nameBarMoved(int n);
    void sl_fontChanged();
    void sl_modelChanged();

protected:
    void resizeEvent(QResizeEvent* e);
    void paintEvent(QPaintEvent* e);
    void keyPressEvent (QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void focusOutEvent(QFocusEvent* fe);
    void focusInEvent(QFocusEvent* fe);
    void wheelEvent (QWheelEvent * we);
    //todo context menu?

public:
    void drawContent(QPainter& p);

private:
    bool isRowInSelection(int row);
    void updateActions();
    void buildMenu(QMenu* m);
    void updateScrollBar();
    void updateSelection(int newSeqNum);
    void moveSelectedRegion( int shift );
    void drawAll();
    void drawSelection(QPainter& p);
    void drawSequenceItem(QPainter& p, int n, bool selected);
    void drawSequenceItem(QPainter& p, int s, bool selected, const U2Region& yRange, int pos);
    void drawFocus(QPainter& p);
    QFont getFont(bool selected) const;
    QRect calculateTextRect(const U2Region& yRange, bool selected) const;
    QRect calculateButtonRect(const QRect& itemRect) const;
    int getSelectedRow() const;
    
    QObject*            labels; // used in GUI tests
    MSAEditor*          editor;
    MSAEditorUI*        ui;
    QScrollBar*         nhBar;
    int                 curSeq;
    QPoint              origin;
    bool                scribbling,shifting;

    QRubberBand*        rubberBand;
    QAction*            editSequenceNameAction;
    QAction*            copyCurrentSequenceAction;
    QAction*            removeCurrentSequenceAction;
    QPixmap*            cachedView;
    bool                completeRedraw;
};

}//namespace
#endif
