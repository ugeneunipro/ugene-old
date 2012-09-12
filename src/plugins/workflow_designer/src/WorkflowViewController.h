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

#ifndef _U2_WORKFLOW_VIEW_CONTROLLER_H_
#define _U2_WORKFLOW_VIEW_CONTROLLER_H_

#include <U2Lang/ActorModel.h>
#include <U2Lang/Aliasing.h>
#include <U2Lang/Schema.h>
#include <U2Lang/WorkflowRunTask.h>

#include <U2Gui/MainWindow.h>
#include <U2Gui/ScriptEditorDialog.h>

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

    WorkflowView* getController() const { return controller; }

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

    void setupLinkCtxMenu(const QString& href, Actor* actor, const QPoint& pos);

    const QList<PortAlias> &getPortAliases() const;
    bool addPortAlias(const PortAlias &alias);
    void setPortAliases(const QList<PortAlias> &aliases);

    bool isIterated() const;
    /*
     * If it isn't iterated then there is one iteration that isn't shown to a user.
     * This hidden iteration is @defaultIteration.
     */
    void setIterated(bool iterated, const Iteration &defaultIteration = Iteration(tr("Default iteration")));

    QString getTypeName() const;
    void setTypeName(const QString &typeName);
    
public slots:
    void sl_deleteItem();
    void sl_selectAll();
    void sl_deselectAll();
    void sl_reset();
    void setLocked(bool b) {locked = b;}
    void setModified(bool b);
    void centerView();
    void setHint(int i) {hint=i; update();}
    void sl_openDocuments();
    void sl_updateDocs() {
        emit configurationChanged();
    }
    void sl_refreshBindings();
    void connectConfigurationEditors();

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

    bool refreshGrouperSlots(Actor *proc);

private:
    WorkflowView* controller;
    bool modified, locked;
    QList<Iteration> iterations;

    WorkflowAbstractRunner* runner;
    int hint;
    QAction* openDocumentsAction;
    QList<PortAlias> portAliases;
    // if it isn't iterated then there are one iteration and it isn't shown to a user
    bool iterated;
    QString includedTypeName;
    
}; // WorkflowScene

class WorkflowView : public MWMDIWindow {
    friend class WorkflowScene;
    Q_OBJECT
public:
    WorkflowView(WorkflowGObject* go);
    ~WorkflowView();

    //WorkflowDesignerService* getService() const {return service;}

    WorkflowEditor* getPropertyEditor() const { return propertyEditor; }

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
public slots:
    void sl_updateUi();
private slots:
    void sl_editItem();
    void sl_onSelectionChanged();
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
    void sl_onSceneLoaded();

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
    void sl_iterationsMode();
    void sl_configureIterations();
    void sl_configureParameterAliases();
    void sl_configurePortAliases();
    void sl_importSchemaToElement();

    void sl_toggleLock(bool = true);
    void sl_createScript();
    void sl_editScript();
    void sl_externalAction();
    void sl_appendExternalToolWorker();
    void sl_protoDeleted(const QString& id);
    void sl_xmlSchemaLoaded(Task*);
    void sl_editExternalTool();
    void sl_findPrototype();    
protected:
    bool onCloseEvent();

private:
    void remapActorsIds(QList<Iteration> &lst);
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
    QAction* externalToolAction;
    QAction* appendExternalTool;
    QAction* editExternalToolAction;
    
    QAction* iterationModeAction;
    QAction* configureIterationsAction;
    QAction* configureParameterAliasesAction;
    QAction* configurePortAliasesAction;
    QAction* importSchemaToElement;
    QAction* runAction;
    QAction* stopAction;
    QAction* validateAction;

    QAction* findPrototypeAction;

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
