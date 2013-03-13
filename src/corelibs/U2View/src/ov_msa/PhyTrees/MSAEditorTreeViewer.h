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

#ifndef _U2_MSAEditor_TREE_VIEWER_H_
#define _U2_MSAEditor_TREE_VIEWER_H_

#include <U2View/TreeViewer.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <QtGui/QGraphicsLineItem>
#include <QtCore/QMap>
#include <U2Algorithm/CreatePhyTreeSettings.h>

namespace U2 {

typedef QMap<QString, QColor> GroupColorSchema;

class MSAEditorTreeViewerUI;


class GroupColors {
public:
    GroupColors();
    QColor getColor(int index);
private:
    QList<QColor> colors;
};

class MSAEditorTreeViewer: public TreeViewer {
    Q_OBJECT
public:
    MSAEditorTreeViewer(const QString& viewName, GObject* obj, GraphicsRectangularBranchItem* root, qreal scale);

    const CreatePhyTreeSettings& getCreatePhyTreeSettings() {return buildSettings;}
    const QString& getParentAlignmentName() {return alignmentName;}
    virtual OptionsPanel* getOptionsPanel(){return 0;}

    void setCreatePhyTreeSettings(const CreatePhyTreeSettings& _buildSettings) {buildSettings = _buildSettings;}
    void setParentAignmentName(const QString& _alignmentName) {alignmentName = _alignmentName;}

    void setTreeVerticalSize(int size);

    QAction* getSortSeqsAction() const {return sortSeqAction;}
protected:
    virtual QWidget* createWidget();
private:
    QAction*              sortSeqAction;
    QString               alignmentName;
    CreatePhyTreeSettings buildSettings;
};

class MSAEditorTreeViewerUI: public TreeViewerUI {
    Q_OBJECT

public:
    MSAEditorTreeViewerUI(MSAEditorTreeViewer* treeViewer);
    ~MSAEditorTreeViewerUI(){}

    QStringList* getOrderedSeqNames();

    const QStringList* getVisibleSeqsList();

    U2Region getTreeSize(); 
    void setTreeVerticalSize(int size);

    virtual void setTreeLayout(TreeLayout newLayout);
protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *me);
    virtual void mouseReleaseEvent(QMouseEvent *e);

    virtual void onLayoutChanged(const TreeLayout& layout);
signals:
    void si_collapseModelChangedInTree(const QStringList* visibleSeqs);
    void si_seqOrderChanged(QStringList* order);
    void si_treeZoomedIn();
    void si_treeZoomedOut();
    void si_groupColorsChanged(const GroupColorSchema& schema);

private slots:
    void sl_selectionChanged(const QStringList& selection);
    void sl_sequenceNameChanged(QString prevName, QString newName);
    virtual void sl_collapseTriggered();
    virtual void sl_zoomToSel();
    virtual void sl_zoomOut();
    void sl_onHeightChanged(int height);
    void sl_onReferenceSeqChanged(const QString &str);
    void sl_sortAlignment();

private:
    void highlightBranches();

    QGraphicsLineItem* subgroupSelector;
    qreal              subgroupSelectorPos;
    bool               subgroupSelectionMode;
    GroupColors        groupColors;

    bool isSinchronized;
    bool curLayoutIsRectangular;

    const MSAEditorTreeViewer* curMSATreeViewer;
};

}//namespace
#endif
