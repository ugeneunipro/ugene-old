/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMenu>
#include <QtGui/QScrollBar>
#include <QtGui/QRubberBand>
#else
#include <QtWidgets/QMenu>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QRubberBand>
#endif

#include "PhyTrees/MSAEditorTreeViewer.h"

namespace U2 {


class MSAEditor;
class MSAEditorUI;
class GObjectView;
class MAlignment;
class MAlignmentModInfo;
class MSAEditorSelection;

class U2VIEW_EXPORT MSAEditorNameList: public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MSAEditorNameList)
public:
    MSAEditorNameList(MSAEditorUI* ui, QScrollBar* nhBar);
    virtual ~MSAEditorNameList();

    QPixmap drawNames(const QList<qint64>& seqIdx, bool drawSelection = false);

private slots:
    virtual void sl_buildStaticMenu(GObjectView* v, QMenu* m);
    virtual void sl_buildContextMenu(GObjectView* v, QMenu* m);
    void sl_copyCurrentSequence();
    void sl_editSequenceName();
    void sl_lockedStateChanged();
    void sl_removeCurrentSequence();
    void sl_selectReferenceSequence();
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&);
    void sl_onScrollBarActionTriggered( int scrollAction );
    void sl_referenceSeqChanged(qint64);

    void sl_startChanged(const QPoint& p, const QPoint& prev);
    void sl_selectionChanged(const MSAEditorSelection& current, const MSAEditorSelection& prev);

    void sl_nameBarMoved(int n);
    void sl_fontChanged();
    void sl_modelChanged();

    void sl_onGroupColorsChanged(const GroupColorSchema&);
protected:
    void updateContent();

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
    int getSelectedRow() const;
    virtual QString getTextForRow(int s);
    virtual QString getSeqName(int s);

    bool                completeRedraw;

public:
    void drawContent(QPainter& p);
    qint64 sequenceIdAtPos(QPoint p);
    void clearGroupsSelections();

signals:
    void si_sequenceNameChanged(QString prevName, QString newName);
    void si_startMsaChanging();
    void si_stopMsaChanging(bool modified);

private:
    bool isRowInSelection(int row);
    void updateActions();
    void buildMenu(QMenu* m);
    void updateScrollBar();
    void updateSelection(int newSeqNum);
    void moveSelectedRegion( int shift );
    void drawAll();
    void drawSelection(QPainter& p);
    void drawSequenceItem(QPainter& p, int row, const QString& text, bool selected);
    void drawSequenceItem(QPainter& p, int s, bool selected);
    void drawSequenceItem(QPainter& p, int s, const QString& name, bool selected, const U2Region& yRange, int pos);
    virtual void drawRefSequence(QPainter &p, QRect r);
    void drawFocus(QPainter& p);
    QFont getFont(bool selected) const;
    QRect calculateTextRect(const U2Region& yRange, bool selected) const;
    QRect calculateButtonRect(const QRect& itemRect) const;

    QObject*            labels; // used in GUI tests
    MSAEditorUI*        ui;
    QScrollBar*         nhBar;
    int                 curSeq;
    int                 startSelectingSeq;
    QPoint              origin;
    bool                scribbling;
    bool                shifting;
    bool                singleSelecting;
    GroupColorSchema    groupColors;

    QRubberBand*        rubberBand;
    QAction*            editSequenceNameAction;
    QAction*            copyCurrentSequenceAction;
    QAction*            removeCurrentSequenceAction;
    QPixmap*            cachedView;

    static const int CROSS_SIZE = 9;
    static const int CHILDREN_OFFSET = 8;
protected:
    MSAEditor*          editor;

};

}//namespace
#endif
