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


#include <U2Gui/ObjectViewModel.h>
#include <U2Core/U2Region.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <U2Core/PhyTree.h>
#include <U2Core/U2OpStatus.h>

#include <QtGui/QMenu>
#include <QtGui/QSplitter>

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
class U2SequenceObject;

#define MSAE_MENU_COPY          "MSAE_MENU_COPY"
#define MSAE_MENU_EDIT          "MSAE_MENU_EDIT"
#define MSAE_MENU_EXPORT        "MSAE_MENU_EXPORT"
#define MSAE_MENU_VIEW          "MSAE_MENU_VIEW"
#define MSAE_MENU_ALIGN         "MSAE_MENU_ALIGN"
#define MSAE_MENU_TREES         "MSAE_MENU_TREES"
#define MSAE_MENU_STATISTICS    "MSAE_MENU_STATISTICS"
#define MSAE_MENU_ADVANCED      "MSAE_MENU_ADVANCED"
#define MSAE_MENU_LOAD          "MSAE_MENU_LOAD_SEQ"

class U2VIEW_EXPORT MSAEditor : public GObjectView {
    Q_OBJECT

    friend class OpenSavedMSAEditorTask;

public:
    MSAEditor(const QString& viewName, GObject* obj);
    ~MSAEditor();

    virtual void buildStaticToolbar(QToolBar* tb);

    virtual void buildStaticMenu(QMenu* m);

    virtual Task* updateViewTask(const QString& stateName, const QVariantMap& stateData);

    virtual QVariantMap saveState();
    
    MAlignmentObject* getMSAObject() const {return msaObject;}

    const MSAEditorUI* getUI() const {return ui;}

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

    static const float zoomMult;
   
signals:
    void si_fontChanged(const QFont& f);
    void si_zoomOperationPerformed(bool resizeModeChanged);


protected slots:
    void sl_onContextMenuRequested(const QPoint & pos);
    void sl_zoomIn(); 
    void sl_zoomOut(); 
    void sl_zoomToSelection();
    void sl_changeFont(); 
    void sl_resetZoom(); 
    void sl_buildTree();
    void sl_openTree();
    void sl_align();
    
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

    CreatePhyTreeSettings settings;
    PhyTreeGeneratorLauncherTask* treeGeneratorTask;
};

// U2VIEW_EXPORT: GUITesting uses MSAEditorUI
class U2VIEW_EXPORT MSAEditorUI : public QWidget {
    Q_OBJECT
    //todo: make public accessors:
    friend class MSALabelWidget;
    friend class MSAEditorSequenceArea;
    friend class MSAEditorConsensusArea;
    friend class MSAEditorNameList;
    friend class MSAEditorTreeViewer;
    friend class MSAEditor;

public:
    MSAEditorUI(MSAEditor* editor);

    QWidget* createLabelWidget(const QString& text = QString(), Qt::Alignment ali = Qt::AlignCenter) const;

    MSAEditor* getEditor() const {return editor;}
    QAction* getUndoAction() const;
    QAction* getRedoAction() const;
    QAction* getCopySelectionAction() const {return copySelectionAction;};

    bool isCollapsibleMode() const { return collapsibleMode; }
    void setCollapsibleMode(bool collapse) { collapsibleMode = collapse; }
    MSACollapsibleItemModel* getCollapseModel() const { return collapseModel; }

public slots:
    void sl_saveScreenshot();
    void sl_saveSvgImage();

private:
    MSAEditor*                      editor;
    MSAEditorNameList*              nameList;
    MSAEditorSequenceArea*          seqArea;
    MSAEditorConsensusArea*         consArea;
    MSAEditorOffsetsViewController* offsetsView;
    MSAEditorStatusWidget*          statusWidget;
    MSAEditorTreeViewer*            treeViewer;
    QSplitter*                      splitter;

    QList<QWidget*>                 nameAreaWidgets;
    QList<QWidget*>                 lw1Widgets;
    QList<QWidget*>                 seqAreaWidgets;
    QList<QWidget*>                 lw2Widgets;
    QList<QWidget*>                 treeAreaWidgets;
    MSAEditorUndoFramework*         undoFWK;
    MSACollapsibleItemModel*        collapseModel;
    bool                            collapsibleMode;

    QAction                         *copySelectionAction;
};



class MSALabelWidget : public QWidget {
    Q_OBJECT
public:
    MSALabelWidget(const MSAEditorUI* _ui, const QString & _t, Qt::Alignment _a);
    
    const MSAEditorUI*  ui;
    QString             text;
    Qt::Alignment       ali;

protected slots:
    void sl_fontChanged();
    

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
};


}//namespace;

#endif
