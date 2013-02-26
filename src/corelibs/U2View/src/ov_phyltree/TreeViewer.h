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
//#include <U2View/TreeOptionsWidget.h>


#include <QtGui/QTransform>

namespace U2 {

class GObjectView;
class TreeViewerUI;
class GraphicsBranchItem;
class GraphicsButtonItem;
class GraphicsRectangularBranchItem;
class CreateBranchesTask;

struct TreeLabelsSettings {
public:
    TreeLabelsSettings()
        : showNames(true), showDistances(true), alignLabels(false) {}

    bool showNames;
    bool showDistances;
    bool alignLabels;
};

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
    virtual OptionsPanel* getOptionsPanel(){return optionsPanel;};


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

    qreal getHorizontalZoom() const;
    void setHorizontalZoom(qreal z);

    qreal getVerticalZoom() const;
    void setVerticalZoom(qreal z);

    QTransform getTransform() const;
    void setTransform(const QTransform& m);

    QVariantMap getSettingsState() const;
    void setSettingsState(const QVariantMap& m);

    void setAlignment(Qt::Alignment alignment);

    TreeViewerUI*       ui;

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
    virtual ~TreeViewerUI();

    static const qreal ZOOM_COEF;
    static const qreal MINIMUM_ZOOM;
    static const qreal MAXIMUM_ZOOM;
    static const int MARGIN;
    static const qreal SIZE_COEF;

    BranchSettings getBranchSettings() const;
    ButtonSettings getButtonSettings() const;
    TextSettings getTextSettings() const;
    TreeSettings getTreeSettings() const;
    TreeLabelsSettings getLabelsSettings() const;

    void updateSettings(const BranchSettings &settings);
    void updateSettings(const ButtonSettings &settings);
    void updateSettings(const TextSettings &settings);
    void updateSettings(const TreeSettings &settings);
    void updateSettings(const TreeLabelsSettings &settings);

    //qreal getZoom() const {return zoom;}
    //void setZoom(qreal z) {zoom = z;}

    qreal getHorizontalZoom() const {return horizontalScale;}
    void setHorizontalZoom(qreal z) {horizontalScale = z;}

    qreal getVerticalZoom() const {return verticalScale;}
    void setVerticalZoom(qreal z) {verticalScale = z;}


    QVariantMap getSettingsState() const;
    void setSettingsState(const QVariantMap& m);


    enum TreeLayout {
        TreeLayout_Rectangular,
        TreeLayout_Circular,
        TreeLayout_Unrooted,
    };

    virtual void setTreeLayout(TreeLayout newLayout);
    TreeLayout getTreeLayout() const;
    bool layoutIsRectangular() const {return TreeLayout_Rectangular == layout;}

protected:
    virtual void wheelEvent(QWheelEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

    GraphicsBranchItem* getRoot() {return root;}
    GraphicsRectangularBranchItem* getRectRoot() {return rectRoot;}
    void zooming(qreal newZoom);
    void zooming(qreal horizontalZoom, qreal verticalZoom);
    void defaultZoom();
protected slots:
    virtual void sl_swapTriggered();
    virtual void sl_collapseTriggered();
    virtual void sl_zoomToSel();
    virtual void sl_zoomOut();
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
    void sl_textSettingsTriggered();
    void sl_treeSettingsTriggered();

    void sl_setSettingsTriggered();
    void sl_branchSettings();
    void sl_zoomToAll();

private:
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

    void setScale(qreal s) { view_scale = s;}
    qreal getScale() {return view_scale;}

    void updateActionsState();

    qreal avgWidth();

    void updateLabelsAlignment(bool on);
    
    PhyTreeObject*      phyObject;
    GraphicsBranchItem* root;
    int                 maxNameWidth;
    qreal               verticalScale;
    qreal               horizontalScale;
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
    TreeLabelsSettings  labelsSettings;
protected:
    GraphicsRectangularBranchItem* rectRoot;
};


}//namespace
#endif
