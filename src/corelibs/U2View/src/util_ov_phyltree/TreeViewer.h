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

#ifndef _U2_TREE_VIEWER_H_
#define _U2_TREE_VIEWER_H_

#include <QtGui/QGraphicsView>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QScrollBar>
#include <QtCore/QMap>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/PhyTree.h>
#include <U2Core/Task.h>
#include <U2Algorithm/PhyTreeGeneratorTask.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <U2Gui/ObjectViewModel.h>

#include <U2View/BranchSettingsDialog.h>
#include <U2View/ButtonSettingsDialog.h>
#include <U2View/TextSettingsDialog.h>
#include <U2View/TreeSettingsDialog.h>


#include <QtGui/QTransform>

namespace U2 {

class GObjectView;
class TreeViewerUI;
class GraphicsBranchItem;
class GraphicsButtonItem;
class GraphicsRectangularBranchItem;
class CreateBranchesTask;

class TreeViewer: public GObjectView {
    Q_OBJECT
public:
    TreeViewer(const QString& viewName, GObject* obj, GraphicsRectangularBranchItem* root, qreal scale);

    //from GObjectView
    virtual void buildStaticToolbar(QToolBar* tb);
    virtual void buildStaticMenu(QMenu* m);

    void createActions();

    virtual QVariantMap saveState();
    virtual Task* updateViewTask(const QString& stateName, const QVariantMap& stateData);


    QAction* getPrintAction() const { return printAction; }
    QAction* getContAction() const { return contAction; }
    QAction* getNameLabelsAction() const { return nameLabelsAction; }
    QAction* getDistanceLabelsAction() const { return distanceLabelsAction; }
    QAction* getCaptureTreeAction() const { return captureTreeAction; }
    QAction* getExportAction() const { return exportAction; }
    QAction* getRectangularLayoutAction() const { return rectangularLayoutAction; }
    QAction* getCircularLayoutAction() const { return circularLayoutAction; }
    QAction* getUnrootedLayoutAction() const { return unrootedLayoutAction; }
    QAction* getTextSettingsAction() const { return textSettingsAction; }
    QAction* getTreeSettingsAction() const { return treeSettingsAction; }
    QAction* getBranchesSettingsAction() const { return branchesSettingsAction; }
    QAction* getZoomToSelAction() const { return zoomToSelAction; }
    QAction* getZoomOutAction() const { return zoomOutAction; }
    QAction* getZoomToAllAction() const { return zoomToAllAction; }

    PhyTreeObject* getPhyObject() const { return phyObject; }
    GraphicsRectangularBranchItem* getRoot() const { return root; }
    void setRoot(GraphicsRectangularBranchItem* rectRoot) { root = rectRoot;}

    qreal getScale() const {return scale;}
    void setScale(qreal scale) {this->scale = scale;}

    qreal getZoom() const;
    void setZoom(qreal z);

    QTransform getTransform() const;
    void setTransform(const QTransform& m);

    QVariantMap getSettingsState() const;
    void setSettingsState(const QVariantMap& m);

protected:
    virtual QWidget* createWidget();
    virtual void onObjectRenamed(GObject* obj, const QString& oldName);

private:
    QAction*            treeSettingsAction;

    QActionGroup*       layoutGroup;
    QAction*            rectangularLayoutAction;
    QAction*            circularLayoutAction;
    QAction*            unrootedLayoutAction;

    QAction*            branchesSettingsAction;

    QAction*            nameLabelsAction;
    QAction*            distanceLabelsAction;
    QAction*            textSettingsAction;
    QAction*            contAction;

    QAction*            zoomToSelAction;
    QAction*            zoomToAllAction;
    QAction*            zoomOutAction;

    QAction*            printAction;
    QAction*            captureTreeAction;
    QAction*            exportAction;

    TreeViewerUI*       ui;
    QByteArray          state;
    PhyTreeObject*      phyObject;
    GraphicsRectangularBranchItem* root;
    qreal               scale;

    void setupLayoutSettingsMenu(QMenu* m);
    void setupShowLabelsMenu(QMenu* m);
    void setupCameraMenu(QMenu* m);
};

class TreeViewerUI: public QGraphicsView {
    Q_OBJECT

public:
    TreeViewerUI(TreeViewer* treeViewer);
    ~TreeViewerUI();

    static const qreal ZOOM_COEF;
    static const qreal MINIMUM_ZOOM;
    static const qreal MAXIMUM_ZOOM;
    static const int MARGIN;
    static const qreal SIZE_COEF;

    BranchSettings getBranchSettings() const;
    ButtonSettings getButtonSettings() const;
    TextSettings getTextSettings() const;
    TreeSettings getTreeSettings() const;

    void updateSettings(const BranchSettings &settings);
    void updateSettings(const ButtonSettings &settings);
    void updateSettings(const TextSettings &settings);
    void updateSettings(const TreeSettings &settings);

    qreal getZoom() const {return zoom;}
    void setZoom(qreal z) {zoom = z;}

    QVariantMap getSettingsState() const;
    void setSettingsState(const QVariantMap& m);

protected:
    virtual void wheelEvent(QWheelEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

private slots:
    void sl_printTriggered();
    void sl_captureTreeTriggered();
    void sl_contTriggered(bool on);
    void sl_exportTriggered();
    void sl_showNameLabelsTriggered(bool on);
    void sl_showDistanceLabelsTriggered(bool on);
    void sl_rectangularLayoutTriggered();
    void sl_circularLayoutTriggered();
    void sl_unrootedLayoutTriggered();
    void sl_layoutRecomputed();
    void sl_chrootTriggered();
    void sl_swapTriggered();
    void sl_collapseTriggered();
    void sl_textSettingsTriggered();
    void sl_treeSettingsTriggered();

    void sl_setSettingsTriggered();
    void sl_branchSettings();
    void sl_zoomToSel();
    void sl_zoomOut();
    void sl_zoomToAll();

private:
    enum TreeLayout {
        TreeLayout_Rectangular,
        TreeLayout_Circular,
        TreeLayout_Unrooted,
    };
    enum LabelType {
        LabelType_SequnceName = 1,
        LabelType_Distance = 2,
    };
    typedef QFlags<LabelType> LabelTypes;

    void updateRect();
    void paint(QPainter &painter);
    void showLabels(LabelTypes labelTypes);
    void addLegend(qreal scale);
    void collapseSelected();

    void updateSettings();

    void updateLayout();

    void updateTextSettings();
    void updateTreeSettings();

    void redrawRectangularLayout();
    bool isSelectedCollapsed();

    void zooming(qreal newzoom);
    void defaultZoom();

    void setScale(qreal s) { view_scale = s;}
    qreal getScale() {return view_scale;}

    void updateActionsState();

    qreal avgWidth();

    void updateLabelsAlignment(bool on);
    
    PhyTreeObject*      phyObject;
    GraphicsBranchItem* root;
    GraphicsRectangularBranchItem* rectRoot;
    bool                contEnabled;
    int                 maxNameWidth;
    bool                showNameLabels;
    bool                showDistanceLabels;
    qreal               zoom;
    qreal               view_scale;
    TreeLayout          layout;
    CreateBranchesTask* layoutTask;
    QGraphicsItem*      legend;
    QMenu*              buttonPopup;

    const TreeViewer*   curTreeViewer;

    QAction*            swapAction;
    QAction*            zoomToAction;
    QAction*            zoomOutAction;
    QAction*            zoomToAllAction;
    QAction*            collapseAction;
    QAction*            setColorAction;
    QAction*            captureAction;
    QAction*            exportAction;

    BranchSettings      branchSettings;
    ButtonSettings      buttonSettings;
    TextSettings        textSettings;
    TreeSettings        treeSettings;
};

}//namespace
#endif
