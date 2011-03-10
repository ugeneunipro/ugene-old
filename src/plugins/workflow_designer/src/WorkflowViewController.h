#ifndef _U2_WORKFLOW_VIEW_CONTROLLER_H_
#define _U2_WORKFLOW_VIEW_CONTROLLER_H_

#include <U2Lang/ActorModel.h>
#include <U2Lang/Schema.h>
#include <U2Lang/WorkflowRunTask.h>

#include <U2Gui/MainWindow.h>
#include <U2Misc/ScriptEditorDialog.h>

#include <QtGui/QAction>
#include <QtGui/QGraphicsScene>

class QComboBox;
class QSplitter;
class QDomDocument;
class QListWidget;
class QListWidgetItem;
class QTabWidget;

namespace U2 {
using namespace Workflow;
class WorkflowView;
class WorkflowScene;
class WorkflowPalette;
class WorkflowEditor;
class WorkflowGObject;
class WorkflowRunTask;

class WorkflowScene : public QGraphicsScene {
    Q_OBJECT
public:
    WorkflowScene(WorkflowView *parent = 0);
    virtual ~WorkflowScene();
    bool isModified() const {return modified;} //FIXME draft support
    bool isLocked() const {return locked;}

    WorkflowAbstractRunner* getRunner() const {return runner;}
    void setRunner(WorkflowAbstractRunner* r) {runner = r;}

    QList<Actor*> getSelectedProcItems() const;
    QList<Actor*> getAllProcs() const;
    
    Actor* getActor(ActorId) const;

    Schema getSchema() const;
    QList<Iteration> getIterations() const {return iterations;}
    void setIterations(const QList<Iteration>& lst);

    void addProcess(Actor*, const QPointF&);
    Actor * createActor( ActorPrototype * proto, const QVariantMap & params = QVariantMap() );
    void clearScene();
    
public slots:
    void sl_deleteItem();
    void sl_selectAll();
    void sl_deselectAll();
    void sl_reset();
    void setLocked(bool b) {locked = b;}
    void setModified(bool b);
    void centerView();
    void setHint(int i) {hint=i; update();}

signals:
    void processItemAdded();
    void processDblClicked();
    void configurationChanged();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent * contextMenuEvent);

    void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
    void dropEvent(QGraphicsSceneDragDropEvent * event);

    void drawBackground(QPainter * painter, const QRectF & rect );

private:
    WorkflowView* controller;
    bool modified, locked;
    QList<Iteration> iterations;

    WorkflowAbstractRunner* runner;
    int hint;
    
}; // WorkflowScene

class WorkflowView : public MWMDIWindow {
    friend class WorkflowScene;
    Q_OBJECT
public:
    WorkflowView(WorkflowGObject* go);
    ~WorkflowView();

    //WorkflowDesignerService* getService() const {return service;}

    virtual void setupMDIToolbar(QToolBar* tb);
    virtual void setupViewMenu(QMenu* n);
    void setupContextMenu(QMenu* menu);

    bool confirmModified();

    ActorPrototype* selectedProto() const {
        return currentProc;
    }

    Actor* getActor() {
        return currentActor;
    }

    WorkflowScene* getScene() const {return scene;}
    Workflow::Metadata getMeta() const {return meta;}
    
    void refreshView() {sl_refreshActorDocs();}
    
    /**
    * Schema can run on local and remote machine
    * if it is run on remote machine then all needed files on local filesystem
    * are saved with schema and passed to remote machine (see WorkflowRemoteRunTask for details)
    */
    enum RunMode {
        LOCAL_HOST,
        REMOTE_MACHINE
    }; // RunMode
private slots:
    void sl_editItem();
    void sl_showEditor();
    void sl_selectProcess(Workflow::ActorPrototype*);
    void sl_procItemAdded();
    void sl_rescaleScene(const QString &scale);
    void sl_exportScene();
    void sl_saveScene();
    void sl_saveSceneAs();
    void sl_loadScene();
    void sl_newScene();
    void sl_updateTitle();

    void sl_copyItems();
    void sl_cutItems();
    void sl_pasteItems(const QString& = QString());
    void sl_pasteSample(const QString&);

    void sl_setStyle();
    void sl_toggleStyle();
    void sl_refreshActorDocs();
    
    void sl_setRunMode();
    void sl_changeScriptMode();
    
    bool sl_validate(bool notify = true);
    void sl_pickInfo(QListWidgetItem*);
    void sl_launch();
    void sl_stop();
    void sl_configureIterations();
    void sl_configureAliases();

    void sl_toggleLock(bool = true);
    void sl_createScript();
    void sl_editScript();
    void sl_protoDeleted(const QString& id);
    void sl_xmlSchemaLoaded(Task*);
    
protected:
    bool onCloseEvent();

private:
    void createActions();
    void saveState();
    
    void localHostLaunch();
    void remoteLaunch();
    
    WorkflowGObject* go;
    QGraphicsView* sceneView;
    WorkflowScene* scene;
    Workflow::Metadata meta;
    ActorPrototype* currentProc;
    Actor* currentActor;

    QAction* deleteAction;
    QAction* selectAction;
    //QAction* bringToFrontAction;
    //QAction* sendToBackAction;
    QComboBox* sceneScaleCombo;
    QAction* copyAction;
    QAction* pasteAction;
    QAction* cutAction;

    QString lastPaste;
    int pasteCount;

    QAction* exportAction;
    QAction* saveAction;
    QAction* saveAsAction;
    QAction* loadAction;
    QAction* newAction;

    QAction* createScriptAcction;
    QAction* editScriptAction;
    
    QAction* configureIterationsAction;
    QAction* configureAliasesAction;
    QAction* runAction;
    QAction* stopAction;
    QAction* validateAction;

    QList<QAction*> styleActions;
    QList<QAction*> runModeActions;
    QList<QAction*> scriptingActions;
    
    QAction* unlockAction;

    QSplitter*      splitter;
    WorkflowPalette* palette;
    WorkflowEditor* propertyEditor;
    QListWidget*    infoList;
    QSplitter*      infoSplitter;
    QTabWidget*     tabs;
    bool            scriptingMode;
    RunMode runMode;
};

}//namespace


#endif
