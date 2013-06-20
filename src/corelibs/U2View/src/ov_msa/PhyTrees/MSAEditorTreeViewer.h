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
class MSAEditor;

class ColorGenerator {
public:
    ColorGenerator(int countOfColors, qreal lightness);
    void setCountOfColors(int counts);
    QColor getColor(int index);
    int getCountOfColors() {return countOfColors;}
private:
    void generateColors();
    int countOfColors;
    QList<QColor> colors;
    QList<qreal> satValues;
    qreal delta;
    qreal hue;
    qreal lightness;
};

enum SynchronizationMode {
    FullSynchronization,
    OnlySeqsSelection,
    WithoutSynchronization,
};

class MSAEditorTreeViewer: public TreeViewer {
    Q_OBJECT
public:
    MSAEditorTreeViewer(const QString& viewName, GObject* obj, GraphicsRectangularBranchItem* root, qreal scale);

    const CreatePhyTreeSettings& getCreatePhyTreeSettings() {return buildSettings;}
    const QString& getParentAlignmentName() {return alignmentName;}
    virtual OptionsPanel* getOptionsPanel(){return 0;}

    void setCreatePhyTreeSettings(const CreatePhyTreeSettings& _buildSettings);
    void setParentAignmentName(const QString& _alignmentName) {alignmentName = _alignmentName;}

    void setTreeVerticalSize(int size);

    QAction* getSortSeqsAction() const {return sortSeqAction;}
    void setSynchronizationMode(SynchronizationMode newSyncMode);
    SynchronizationMode getSynchronizationMode() const{return syncMode;}


    void setMSAEditor(MSAEditor* _msa) {msa = _msa;}
    MSAEditor* getMsaEditor() const{return msa;}
protected:
    virtual QWidget* createWidget();
private slots:
    void sl_refreshTree();
signals:
    void si_refreshTree(MSAEditorTreeViewer& treeViewer);
private:
    QAction*              refreshTreeAction;
    QAction*              sortSeqAction;
    QString               alignmentName;
    CreatePhyTreeSettings buildSettings;
    MSAEditor*            msa;
    SynchronizationMode   syncMode;
    bool                  slotsAreConnected;
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

    bool canSynchronizeWithMSA(MSAEditor* msa);
    
    void setSynchronizeMode(SynchronizationMode syncMode);
protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *me);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    virtual void onLayoutChanged(const TreeLayout& layout);
    virtual void updateSettings(const TreeSettings &settings);
signals:
    void si_collapseModelChangedInTree(const QStringList* visibleSeqs);
    void si_seqOrderChanged(QStringList* order);
    void si_treeZoomedIn();
    void si_treeZoomedOut();
    void si_groupColorsChanged(const GroupColorSchema& schema);
public slots:
    void sl_sortAlignment();
private slots:
    void sl_selectionChanged(const QStringList& selection);
    void sl_sequenceNameChanged(QString prevName, QString newName);
    virtual void sl_collapseTriggered();
    void sl_onHeightChanged(int height, bool isMinimumSize, bool isMaximumSize);
    void sl_onReferenceSeqChanged(const QString &str);
    void sl_onSceneRectChanged(const QRectF&);
    virtual void sl_rectLayoutRecomputed();
    void sl_onVisibleRangeChanged(QStringList visibleSeqs, int height);

private:
    void highlightBranches();
    QList<GraphicsBranchItem*> getListNodesOfTree();

    QGraphicsLineItem* subgroupSelector;
    qreal              subgroupSelectorPos;
    bool               subgroupSelectionMode;
    ColorGenerator        groupColors;

    bool isSinchronized;
    bool curLayoutIsRectangular;

    MSAEditorTreeViewer* curMSATreeViewer;
    SynchronizationMode syncMode;

    bool hasMinSize;
    bool hasMaxSize;
};

}//namespace
#endif
