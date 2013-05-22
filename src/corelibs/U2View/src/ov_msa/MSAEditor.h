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

#ifndef _U2_MSA_EDITOR_H_
#define _U2_MSA_EDITOR_H_


#include <U2Core/U2Region.h>
#include <U2Core/PhyTree.h>
#include <U2Core/U2OpStatus.h>

#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/DNASequenceObject.h>

#include <U2View/UndoRedoFramework.h>

#include <QtCore/QVariantMap>

#include <QtGui/QMenu>
#include <QtGui/QSplitter>
#include <QtGui/QTabWidget>
#include <U2View/MSAEditorTreeManager.h>

namespace U2 {

class MAlignmentObject;
class PhyTreeObject;
class MSAEditorUI;
class MSAEditorSequenceArea;
class MSAEditorConsensusArea;
class MSAEditorNameList;
class MSAEditorOffsetsViewController;
class MSAEditorStatusWidget;
class MSAEditorUndoFramework;
class PhyTreeGeneratorLauncherTask;
class MSAEditorTreeViewer;
class MSACollapsibleItemModel;
class MSAEditorSimilarityColumn;
class MSADistanceMatrix;
class MSASNPHighligtingScheme;
class SimilarityStatisticsSettings;
class MSAEditorAlignmentDependentWidget;
class TreeViewer;
class MSAEditorMultiTreeViewer;
class PairwiseAlignmentTask;


#define MSAE_MENU_COPY          "MSAE_MENU_COPY"
#define MSAE_MENU_EDIT          "MSAE_MENU_EDIT"
#define MSAE_MENU_EXPORT        "MSAE_MENU_EXPORT"
#define MSAE_MENU_VIEW          "MSAE_MENU_VIEW"
#define MSAE_MENU_ALIGN         "MSAE_MENU_ALIGN"
#define MSAE_MENU_TREES         "MSAE_MENU_TREES"
#define MSAE_MENU_STATISTICS    "MSAE_MENU_STATISTICS"
#define MSAE_MENU_ADVANCED      "MSAE_MENU_ADVANCED"
#define MSAE_MENU_LOAD          "MSAE_MENU_LOAD_SEQ"

class SNPSettings {
public:
    SNPSettings():seqName(QString("")){};
    QPoint clickPoint;
    QString seqName;
};

class PairwiseAlignmentWidgetsSettings {
public:
    PairwiseAlignmentWidgetsSettings() : inNewWindow(true), pairwiseAlignmentTask(NULL),
        showSequenceWidget(true), showAlgorithmWidget(false), showOutputWidget(false), sequenceSelectionModeOn(false) {}
    QString firstSequenceName;
    QString secondSequenceName;
    QString algorithmName;
    bool inNewWindow;
    QString resultFileName;
    PairwiseAlignmentTask* pairwiseAlignmentTask;
    bool showSequenceWidget;
    bool showAlgorithmWidget;
    bool showOutputWidget;
    bool sequenceSelectionModeOn;

    QVariantMap customSettings;
};

class U2VIEW_EXPORT MSAEditor : public GObjectView {
    Q_OBJECT

    friend class OpenSavedMSAEditorTask;
    friend class MSAEditorTreeViewerUI;

public:
    MSAEditor(const QString& viewName, GObject* obj);
    ~MSAEditor();

    virtual void buildStaticToolbar(QToolBar* tb);

    virtual void buildStaticMenu(QMenu* m);

    virtual Task* updateViewTask(const QString& stateName, const QVariantMap& stateData);

    virtual QVariantMap saveState();

    virtual OptionsPanel* getOptionsPanel(){return optionsPanel;};
    
    MAlignmentObject* getMSAObject() const {return msaObject;}

    MSAEditorUI* getUI() const {return ui;}

    int getAlignmentLen() const;
    
    int getNumSequences() const;

    const QRect& getCurrentSelection() const;
    
    const QFont& getFont() const {return font;}
    int getFirstVisibleBase() const;
    float getZoomFactor() const {return zoomFactor;}

    enum ResizeMode {
        ResizeMode_FontAndContent, ResizeMode_OnlyContent
    };

    ResizeMode getResizeMode() const { return resizeMode; }

    int getRowHeight() const;

    int getColumnWidth() const;

    void copyRowFromSequence(U2SequenceObject *seqObj, U2OpStatus &os);
    void createDistanceColumn(MSADistanceMatrix* algo);
    
    static const float zoomMult;

    void setReference(QString ref);

    QString getRefSeqName(){return snp.seqName;};

    PairwiseAlignmentWidgetsSettings* getPairwiseAlignmentWidgetsSettings() { return pairwiseAlignmentWidgetsSettings; }
   
    MSAEditorTreeManager* getTreeManager() {return &treeManager;}

    void buildTree();

    void resetCollapsibleModel();

signals:
    void si_fontChanged(const QFont& f);
    void si_zoomOperationPerformed(bool resizeModeChanged);
    void si_referenceSeqChanged(const QString &str);
    void si_sizeChanged(int newHeight, bool isMinimumSize, bool isMaximumSize);

protected slots:
    void sl_onContextMenuRequested(const QPoint & pos);
    void sl_zoomIn(); 
    void sl_zoomOut(); 
    void sl_zoomToSelection();
    void sl_changeFont(); 
    void sl_resetZoom(); 
    void sl_buildTree();
    void sl_align();
    void sl_setSeqAsRefrence();

    void sl_onSeqOrderChanged(QStringList* order);
    void sl_showTreeOP();
    void sl_hideTreeOP();
    
protected:
    virtual QWidget* createWidget();
    bool eventFilter(QObject* o, QEvent* e);
    virtual void onObjectRenamed(GObject* obj, const QString& oldName);
private:
    void addCopyMenu(QMenu* m);
    void addEditMenu(QMenu* m);
    void addExportMenu(QMenu* m);
    void addViewMenu(QMenu* m);
    void addAlignMenu(QMenu* m);
    void addTreeMenu(QMenu* m);
    void addAdvancedMenu(QMenu* m);
    void addStatisticsMenu(QMenu* m);
    void addLoadMenu(QMenu* m);
    void setFont(const QFont& f);
    void calcFontPixelToPointSizeCoef();
    void updateActions();
    void setFirstVisibleBase(int firstPos);
    void setZoomFactor(float newZoomFactor) {zoomFactor = newZoomFactor;}
    void initDragAndDropSupport();

    MAlignmentObject* msaObject;
    MSAEditorUI*      ui;
    QFont             font;
    ResizeMode        resizeMode;
    float             zoomFactor;
    float             fontPixelToPointSize;  

    QAction*          zoomInAction;
    QAction*          zoomOutAction;
    QAction*          zoomToSelectionAction;
    QAction*          changeFontAction;
    QAction*          resetFontAction;
    QAction*          buildTreeAction;
    QAction*          saveScreenshotAction;
    QAction*          saveSvgAction;
    QAction*          alignAction;
    QAction*          setAsRefrenceSequenceAction;

    QToolBar*         toolbar;

    SNPSettings snp;
    PairwiseAlignmentWidgetsSettings* pairwiseAlignmentWidgetsSettings;
    MSAEditorTreeManager           treeManager;
};

class SinchronizedObjectView : public QObject{
// U2VIEW_EXPORT: GUITesting uses MSAEditorUI
    Q_OBJECT
public:
    SinchronizedObjectView();
    SinchronizedObjectView(QSplitter *_spliter);
    void addSeqArea(MSAEditorSequenceArea* _seqArea) {seqArea = _seqArea;}
    void addObject(QWidget *obj, int index, qreal coef);
    void addObject(QWidget *neighboringWidget, QWidget *obj, qreal coef, int neighboringShift = 0);
    void removeObject(QWidget *obj);
    QSplitter* getSpliter();
private:
    QList<QWidget *>       objects;
    QList<int>             widgetSizes;
    MSAEditorSequenceArea* seqArea;
    QSplitter*             spliter;
};

// U2VIEW_EXPORT: GUITesting uses MSAEditorUI
class U2VIEW_EXPORT MSAEditorUI : public QWidget {

    Q_OBJECT
    //todo: make public accessors:
    friend class MSAWidget;
    friend class MSAEditorSequenceArea;
    friend class MSAEditorConsensusArea;
    friend class MSAEditorNameList;
    friend class MSAEditorTreeViewer;
    friend class MSAEditor;
    friend class MSAEditorSimilarityColumn;

public:
    MSAEditorUI(MSAEditor* editor);

    QWidget* createLabelWidget(const QString& text = QString(), Qt::Alignment ali = Qt::AlignCenter);

    MSAEditor* getEditor() const {return editor;}
    QAction* getUndoAction() const;
    QAction* getRedoAction() const;
    QAction* getCopySelectionAction() const {return copySelectionAction;};

    bool isCollapsibleMode() const { return collapsibleMode; }
    void setCollapsibleMode(bool collapse) { collapsibleMode = collapse; }
    MSACollapsibleItemModel* getCollapseModel() const { return collapseModel; }

    MSAEditorSequenceArea* getSequenceArea() {return seqArea;}
    MSAEditorNameList*     getEditorNameList() {return nameList;}

    void createDistanceColumn(MSADistanceMatrix* algo);

    void addTreeView(GObjectViewWindow* treeView);

    void setSimilaritySettings(const SimilarityStatisticsSettings* settings);

    void showSimilarity();
    void hideSimilarity();
    
    const MSAEditorAlignmentDependentWidget* getSimilarityWidget(){return similarityStatistics;}

    MSAEditorTreeViewer* getCurrentTree() const;

    MSAEditorMultiTreeViewer* getMultiTreeViewer(){return multiTreeViewer;}

    SinchronizedObjectView                      view;

public slots:
    void sl_saveScreenshot();
    void sl_saveSvgImage();
private slots:
    void sl_onTabsCountChanged(int tabsCount);
signals:
    void si_showTreeOP();
    void si_hideTreeOP();

private:
    MSAEditor*                         editor;
    MSAEditorNameList*                 nameList;
    MSAEditorSequenceArea*             seqArea;
    MSAEditorConsensusArea*            consArea;
    MSAEditorOffsetsViewController*    offsetsView;
    MSAEditorStatusWidget*             statusWidget;
    QWidget*                           seqAreaContainer;
    QWidget*                           nameAreaContainer;

    QList<QWidget*>                 nameAreaWidgets;
    QList<QWidget*>                 lw1Widgets;
    QList<QWidget*>                 seqAreaWidgets;
    QList<QWidget*>                 lw2Widgets;
    QList<QWidget*>                 treeAreaWidgets;
    //MSAEditorUndoFramework*         undoFWK;
    MsaUndoRedoFramework*           undoFWK;

    MSACollapsibleItemModel*        collapseModel;
    bool                            collapsibleMode;

    MSAEditorSimilarityColumn*         dataList;
    MSAEditorMultiTreeViewer*          multiTreeViewer;
    MSAEditorAlignmentDependentWidget* similarityStatistics;
    MSAEditorTreeViewer*               treeViewer;

    QAction                         *copySelectionAction;
};

class MSAWidget : public QWidget {
    Q_OBJECT
public:
    MSAWidget(MSAEditorUI* _ui);
    virtual ~MSAWidget() {}
    const QFont& getMsaEditorFont(){return ui->getEditor()->getFont();}
    void setHeightMargin(int _heightMargin);
protected slots:
    void sl_fontChanged();
protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);

    MSAEditorUI*  ui;
    int heightMargin;
};

class MSALabelWidget : public MSAWidget {
    Q_OBJECT
public:
    MSALabelWidget(MSAEditorUI* _ui, const QString & _t, Qt::Alignment _a);
    
    QString             text;
    Qt::Alignment       ali;

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
};




}//namespace;

#endif
