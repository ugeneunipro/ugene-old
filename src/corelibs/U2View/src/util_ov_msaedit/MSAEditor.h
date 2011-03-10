#ifndef _U2_MSA_EDITOR_H_
#define _U2_MSA_EDITOR_H_

#include <U2Gui/ObjectViewModel.h>
#include <U2Core/U2Region.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <U2Core/PhyTree.h>

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
class PhyTreeGeneratorTask;
class MSAEditorTreeViewer;

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

    const QRect getCurrentSelection() const;
    
    const QFont& getFont() const {return font;}
    int getFirstVisibleBase() const;
    float getZoomFactor() const {return zoomFactor;}

    enum ResizeMode {
        ResizeMode_FontAndContent, ResizeMode_OnlyContent
    };

    ResizeMode getResizeMode() const { return resizeMode; }

    int getRowHeight() const;

    int getColumnWidth() const;
    
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
    void sl_saveScreenshot();

protected:
    virtual QWidget* createWidget();

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

    CreatePhyTreeSettings settings;
    PhyTreeGeneratorTask* treeGeneratorTask;
};

class MSAEditorUI : public QWidget {
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
   
    void saveScreenshot(const QString& filename, const QString& format);

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
};


}//namespace;

#endif
