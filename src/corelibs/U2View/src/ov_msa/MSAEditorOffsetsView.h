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

#ifndef _U2_MSA_EDITOR_OFFSETS_VIEW_H_
#define _U2_MSA_EDITOR_OFFSETS_VIEW_H_

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QEvent>

#include <QtGui/QWidget>

namespace U2 {

class MSAEditor;
class MAlignmentObject;
class MAlignment;
class MAlignmentModInfo;
class MSAEditorSequenceArea;
class MSAEditorBaseOffsetCache;
class MSAEditorOffsetsViewWidget;

class MSAEditorOffsetsViewController : public QObject {
    Q_OBJECT
public:
    MSAEditorOffsetsViewController(QObject* p, MSAEditor* editor, MSAEditorSequenceArea* seqArea);

    MSAEditorOffsetsViewWidget* getLeftWidget() const {return lw;}
    MSAEditorOffsetsViewWidget* getRightWidget() const {return rw;}

    QAction* getToggleColumnsViewAction() const {return viewAction;}
    bool eventFilter(QObject* o, QEvent* e);

private slots:
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&){updateOffsets();}
    void sl_startChanged(const QPoint& , const QPoint& ) {updateOffsets();}
    void sl_fontChanged() {updateOffsets();}
    void sl_modelChanged() {updateOffsets();}
    void sl_showOffsets(bool);
private:
    void updateOffsets();
    
    MSAEditorSequenceArea*      seqArea;
    MSAEditor*                  editor;
    MSAEditorOffsetsViewWidget* lw;
    MSAEditorOffsetsViewWidget* rw;
    QAction*                    viewAction;
};

class MSAEditorOffsetsViewWidget : public QWidget {
    friend class MSAEditorOffsetsViewController;
public:
    MSAEditorOffsetsViewWidget(MSAEditor* editor, MSAEditorSequenceArea* seqArea, MSAEditorBaseOffsetCache* cache, bool showStartPos);
    ~MSAEditorOffsetsViewWidget();
    
protected:
    void paintEvent(QPaintEvent* e);
    void updateView();
    void drawAll(QPainter& p);
    QFont getOffsetsFont();

private:
    MSAEditorSequenceArea*      seqArea;
    MSAEditor*                  editor;
    MSAEditorBaseOffsetCache*   cache;
    bool                        showStartPos;
    bool                        completeRedraw;
    QPixmap*                    cachedView;
};

}//namespace;

#endif
