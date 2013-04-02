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


#include <algorithm>
#include <functional>

#include <QtCore/QFileInfo>
#include <QtGui/QBoxLayout>
#include <QtGui/QClipboard>
#include <QtGui/QCloseEvent>
#include <QtGui/QComboBox>
#include <QtGui/QFileDialog>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPrinter>
#include <QtGui/QShortcut>
#include <QtGui/QSplitter>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtSvg/QSvgGenerator>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectService.h>
#include <U2Core/Settings.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Designer/DesignerUtils.h>
#include <U2Designer/GrouperEditor.h>
#include <U2Designer/MarkerEditor.h>
#include <U2Designer/WizardController.h>
#include <U2Gui/ExportImageDialog.h>
#include <U2Gui/GlassView.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>
#include <U2Lang/ActorModel.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/ExternalToolCfg.h>
#include <U2Lang/GrouperSlotAttribute.h>
#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/IncludedProtoFactory.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/MapDatatypeEditor.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowManager.h>
#include <U2Lang/WorkflowRunTask.h>
#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Remote/DistributedComputingUtil.h>
#include <U2Remote/RemoteMachine.h>
#include <U2Remote/RemoteMachineMonitorDialogController.h>
#include <U2Remote/RemoteWorkflowRunTask.h>

#include "ChooseItemDialog.h"
#include "CreateScriptWorker.h"
#include "ImportSchemaDialog.h"
#include "ItemViewStyle.h"
#include "PortAliasesConfigurationDialog.h"
#include "SceneSerializer.h"
#include "SchemaAliasesConfigurationDialogImpl.h"
#include "SchemaConfigurationDialog.h"
#include "WorkflowDesignerPlugin.h"
#include "WorkflowDocument.h"
#include "WorkflowEditor.h"
#include "WorkflowMetaDialog.h"
#include "WorkflowPalette.h"
#include "WorkflowSamples.h"
#include "WorkflowSceneIOTasks.h"
#include "WorkflowViewController.h"
#include "WorkflowViewItems.h"
#include "WorkflowViewItems.h"
#include "library/CreateExternalProcessDialog.h"
#include "library/ExternalProcessWorker.h"
#include "library/ScriptWorker.h" 


/* TRANSLATOR U2::LocalWorkflow::WorkflowView*/

namespace U2 {

#define LAST_DIR SETTINGS + "lastdir"
#define SPLITTER_STATE SETTINGS + "splitter"
#define EDITOR_STATE SETTINGS + "editor"
#define PALETTE_STATE SETTINGS + "palette"
#define TABS_STATE SETTINGS + "tabs"

enum {ElementsTab,SamplesTab};

#define WS 1000
#define MAX_FILE_SIZE 1000000

static QString percentStr = WorkflowView::tr("%");

/********************************
* PercentValidator
********************************/
class PercentValidator : public QRegExpValidator {
public:
    PercentValidator(QObject* parent) : QRegExpValidator(QRegExp("[1-9][0-9]*"+percentStr), parent) {}
    void fixup(QString& input) const {
        if (!input.endsWith(percentStr)) {
            input.append(percentStr);
        }
    }
}; // PercentValidator

static const QString XML_SCHEMA_WARNING = WorkflowView::tr("You opened obsolete schema in XML format. It is strongly recommended"
                                                           " to clear working space and create schema from scratch.");

static const QString XML_SCHEMA_APOLOGIZE = WorkflowView::tr("Sorry! This schema is obsolete and cannot be opened.");

/********************************
* WorkflowView
********************************/
WorkflowView::WorkflowView(WorkflowGObject* go) 
: MWMDIWindow(tr("Workflow Designer")), sceneRecreation(false), go(go), currentProto(NULL), currentActor(NULL), pasteCount(0), 
scriptingMode(false) {
    elementsMenu = NULL;
    schema = new Schema();
    schema->setDeepCopyFlag(true);

    SceneCreator sc(schema, meta);
    scene = sc.createScene(this);
    connect(scene, SIGNAL(processItemAdded()), SLOT(sl_procItemAdded()));
    connect(scene, SIGNAL(processDblClicked()), SLOT(sl_toggleStyle()));
    connect(scene, SIGNAL(selectionChanged()), SLOT(sl_editItem()));
    connect(scene, SIGNAL(selectionChanged()), SLOT(sl_onSelectionChanged()));

    runMode = (RunMode)WorkflowSettings::getRunMode();
    scriptingMode = WorkflowSettings::getScriptingMode();

    palette = new WorkflowPalette(WorkflowEnv::getProtoRegistry()/*, this*/);
    palette->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));
    connect(palette, SIGNAL(processSelected(Workflow::ActorPrototype*)), SLOT(sl_selectPrototype(Workflow::ActorPrototype*)));
    connect(palette, SIGNAL(si_protoDeleted(const QString&)), SLOT(sl_protoDeleted(const QString&)));
    connect(palette, SIGNAL(si_protoListModified()), SLOT(sl_protoListModified()));
    connect(palette, SIGNAL(si_protoChanged()), scene, SLOT(sl_updateDocs()));
    
    infoList = new QListWidget(this);
    connect(infoList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(sl_pickInfo(QListWidgetItem*)));
    splitter = new QSplitter(this);

    tabs = new QTabWidget(this);
    tabs->insertTab(ElementsTab, palette, tr("Elements"));
    SamplesWidget* samples = new SamplesWidget(scene);
    tabs->insertTab(SamplesTab, samples, tr("Samples"));
    splitter->addWidget(tabs);
    sceneView = new GlassView(scene);
    sceneView->setObjectName("sceneView");
    connect(samples, SIGNAL(setupGlass(GlassPane*)), sceneView, SLOT(setGlass(GlassPane*)));
    connect(samples, SIGNAL(sampleSelected(QString)), this, SLOT(sl_pasteSample(QString)));
    connect(tabs, SIGNAL(currentChanged(int)), samples, SLOT(cancelItem()));
    connect(tabs, SIGNAL(currentChanged(int)), palette, SLOT(resetSelection()));
    connect(tabs, SIGNAL(currentChanged(int)), scene, SLOT(setHint(int)));

    propertyEditor = new WorkflowEditor(this);
    propertyEditor->setIterated(scene->isIterated());

    sceneView->setAlignment(Qt::AlignCenter);
    infoSplitter = new QSplitter(Qt::Vertical, splitter);
    infoSplitter->addWidget(sceneView);
    {
        SpecialParametersPanel *specialParameters = new SpecialParametersPanel(propertyEditor);
        propertyEditor->setSpecialPanel(specialParameters);
        QGroupBox* w = new QGroupBox(infoSplitter);
        w->setFlat(true);
        w->setTitle(tr("Error list"));
        QVBoxLayout* vl = new QVBoxLayout(w);
        vl->setSpacing(0);
        vl->setMargin(0);
        vl->setContentsMargins(0,0,0,0);
        vl->addWidget(infoList);
        w->hide();
        specialParameters->hide();
        infoSplitter->addWidget(w);
        infoSplitter->addWidget(specialParameters);
    }
    splitter->addWidget(infoSplitter);
    splitter->addWidget(propertyEditor);

    Settings* settings = AppContext::getSettings();
    if (settings->contains(SPLITTER_STATE)) {
        splitter->restoreState(settings->getValue(SPLITTER_STATE).toByteArray());
    }
    if (settings->contains(PALETTE_STATE)) {
        palette->restoreState(settings->getValue(PALETTE_STATE));
    }
    tabs->setCurrentIndex(settings->getValue(TABS_STATE, SamplesTab).toInt());

    scene->views().at(0)->setDragMode(QGraphicsView::RubberBandDrag);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(splitter);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    createActions();
    sl_changeScriptMode();

    if(go) {
        LoadWorkflowTask::FileFormat format = LoadWorkflowTask::detectFormat(go->getSceneRawData());
        go->setView(this);
        QString err;
        if(format == LoadWorkflowTask::HR) {
            err = HRSchemaSerializer::string2Schema(go->getSceneRawData(), schema, &meta);
        } else if(format == LoadWorkflowTask::XML) {
            QDomDocument xml;
            QMap<ActorId, ActorId> remapping;
            xml.setContent(go->getSceneRawData().toUtf8());
            err = SchemaSerializer::xml2schema(xml.documentElement(), schema, remapping);
            SchemaSerializer::readMeta(&meta, xml.documentElement());
            scene->setModified(false);
            if(err.isEmpty()) {
                QMessageBox::warning(this, tr("Warning!"), XML_SCHEMA_WARNING);
            } else {
                QMessageBox::warning(this, tr("Warning!"), XML_SCHEMA_APOLOGIZE);
            }
        } else {
            coreLog.error(tr("Undefined workflow format for %1").arg(go->getDocument() ? go->getDocument()->getURLString() : tr("file")));
            sl_newScene();
        }
        scene->connectConfigurationEditors();

        if (!err.isEmpty()) {
            sl_newScene();
            coreLog.error(err);
        } else {
            SceneCreator sc(schema, meta);
            sc.recreateScene(scene);
            if (go->getDocument()) {
                meta.url = go->getDocument()->getURLString();
            }
            sl_updateTitle();
            propertyEditor->resetIterations();
            scene->setModified(false);
            sl_refreshActorDocs();
        }
    } else {
        sl_newScene();
    }

    connect(scene, SIGNAL(configurationChanged()), SLOT(sl_refreshActorDocs()));
    connect(propertyEditor, SIGNAL(iterationSelected()), SLOT(sl_refreshActorDocs()));
    connect(WorkflowSettings::watcher, SIGNAL(changed()), scene, SLOT(update()));
    propertyEditor->reset();
}

WorkflowView::~WorkflowView() {
    uiLog.trace("~WorkflowView");
    if(AppContext::getProjectService()) {
        AppContext::getProjectService()->enableSaveAction(true);
    }
    WorkflowSettings::setRunMode((int)runMode);
    WorkflowSettings::setScriptingMode(scriptingMode);
}

void WorkflowView::sl_rescaleScene(const QString &scale)
{
    int percentPos = scale.indexOf(percentStr);
    double newScale = scale.left(percentPos).toDouble() / 100.0;
    QMatrix oldMatrix = scene->views().at(0)->matrix();
    scene->views().at(0)->resetMatrix();
    scene->views().at(0)->translate(oldMatrix.dx(), oldMatrix.dy());
    scene->views().at(0)->scale(newScale, newScale);
    QRectF rect = scene->sceneRect();
    qreal w = rect.width()/newScale;
    qreal h = rect.height()/newScale;
    rect.setWidth(w);
    rect.setHeight(h);
    scene->setSceneRect(rect);
}


void WorkflowView::createActions() {
    runAction = new QAction(tr("&Run schema"), this);
    runAction->setIcon(QIcon(":workflow_designer/images/run.png"));
    runAction->setShortcut(QKeySequence("Ctrl+R"));
    connect(runAction, SIGNAL(triggered()), SLOT(sl_launch()));

    stopAction = new QAction(tr("S&top schema"), this);
    stopAction->setIcon(QIcon(":workflow_designer/images/stopTask.png"));
    connect(stopAction, SIGNAL(triggered()), SLOT(sl_stop()));

    validateAction = new QAction(tr("&Validate schema"), this);
    validateAction->setIcon(QIcon(":workflow_designer/images/ok.png"));
    validateAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(validateAction, SIGNAL(triggered()), SLOT(sl_validate()));

    newAction = new QAction(tr("&New schema"), this);
    newAction->setIcon(QIcon(":workflow_designer/images/filenew.png"));
    newAction->setShortcuts(QKeySequence::New);
    connect(newAction, SIGNAL(triggered()), SLOT(sl_newScene()));

    saveAction = new QAction(tr("&Save schema"), this);
    saveAction->setIcon(QIcon(":workflow_designer/images/filesave.png"));
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setShortcutContext(Qt::WindowShortcut);
    if(AppContext::getProjectService()) {
        AppContext::getProjectService()->enableSaveAction(false);
    }
    connect(saveAction, SIGNAL(triggered()), SLOT(sl_saveScene()));
    
    saveAsAction = new QAction(tr("&Save schema as..."), this);
    saveAsAction->setIcon(QIcon(":workflow_designer/images/filesaveas.png"));
    connect(saveAsAction, SIGNAL(triggered()), SLOT(sl_saveSceneAs()));

    showWizard = new QAction(tr("Show wizard"), this);
    connect(showWizard, SIGNAL(triggered()), SLOT(sl_showWizard()));

    loadAction = new QAction(tr("&Load schema"), this);
    loadAction->setIcon(QIcon(":workflow_designer/images/fileopen.png"));
    loadAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(loadAction, SIGNAL(triggered()), SLOT(sl_loadScene()));

    exportAction = new QAction(tr("&Export schema"), this);
    exportAction->setIcon(QIcon(":workflow_designer/images/export.png"));
    exportAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    connect(exportAction, SIGNAL(triggered()), SLOT(sl_exportScene()));

    deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setIcon(QIcon(":workflow_designer/images/delete.png"));
    deleteAction->setShortcuts(QKeySequence::Delete);
    connect(deleteAction, SIGNAL(triggered()), scene, SLOT(sl_deleteItem()));

    configureParameterAliasesAction = new QAction(tr("Configure parameter aliases..."), this);
    configureParameterAliasesAction->setIcon(QIcon(":workflow_designer/images/table_relationship.png"));
    connect(configureParameterAliasesAction, SIGNAL(triggered()), SLOT(sl_configureParameterAliases()));

    configurePortAliasesAction = new QAction(tr("Configure port and slot aliases..."), this);
    configurePortAliasesAction->setIcon(QIcon(":workflow_designer/images/port_relationship.png"));
    connect(configurePortAliasesAction, SIGNAL(triggered()), SLOT(sl_configurePortAliases()));

    importSchemaToElement = new QAction(tr("Import schema to element..."), this);
    importSchemaToElement->setIcon(QIcon(":workflow_designer/images/import.png"));
    connect(importSchemaToElement, SIGNAL(triggered()), SLOT(sl_importSchemaToElement()));

    iterationModeAction = new QAction(tr("Switch on/off iteration mode"), this);
    iterationModeAction->setIcon(QIcon(":workflow_designer/images/checked_tag.png"));
    iterationModeAction->setCheckable(true);
    iterationModeAction->setChecked(scene->isIterated());
    connect(iterationModeAction, SIGNAL(triggered()), SLOT(sl_iterationsMode()));
    
    configureIterationsAction = new QAction(tr("Configure iterations..."), this);
    configureIterationsAction->setIcon(QIcon(":workflow_designer/images/tag.png"));
    configureIterationsAction->setEnabled(scene->isIterated());
    connect(configureIterationsAction, SIGNAL(triggered()), SLOT(sl_configureIterations()));

    connect(iterationModeAction, SIGNAL(toggled(bool)), configureIterationsAction, SLOT(setEnabled(bool)));

    selectAction = new QAction(tr("Select all elements"), this);
    selectAction->setShortcuts(QKeySequence::SelectAll);
    connect(selectAction, SIGNAL(triggered()), scene, SLOT(sl_selectAll()));

    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setIcon(QIcon(":workflow_designer/images/editcopy.png"));
    copyAction->setShortcut(QKeySequence("Ctrl+C"));
    connect(copyAction, SIGNAL(triggered()), SLOT(sl_copyItems()));

    cutAction = new QAction(tr("Cu&t"), this);
    cutAction->setIcon(QIcon(":workflow_designer/images/editcut.png"));
    cutAction->setShortcuts(QKeySequence::Cut);
    connect(cutAction, SIGNAL(triggered()), SLOT(sl_cutItems()));

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setIcon(QIcon(":workflow_designer/images/editpaste.png"));
    pasteAction->setShortcuts(QKeySequence::Paste);
    connect(pasteAction, SIGNAL(triggered()), SLOT(sl_pasteItems()));

    sceneScaleCombo = new QComboBox(this);
    sceneScaleCombo->setEditable(true);
    sceneScaleCombo->setValidator(new PercentValidator(this));
    QStringList scales;
    scales << "25%" << "50%" << "75%" << "100%" << "125%" << "150%" << "200%";
    sceneScaleCombo->addItems(scales);
    sceneScaleCombo->setCurrentIndex(3);
    connect(sceneScaleCombo, SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_rescaleScene(const QString &)));

    QAction* simpleStyle = new QAction(tr("Minimal"), this);
    simpleStyle->setData(QVariant(ItemStyles::SIMPLE));
    styleActions << simpleStyle;
    connect(simpleStyle, SIGNAL(triggered()), SLOT(sl_setStyle()));

    QAction* extStyle = new QAction(tr("Extended"), this);
    extStyle->setData(QVariant(ItemStyles::EXTENDED));
    styleActions << extStyle;
    connect(extStyle, SIGNAL(triggered()), SLOT(sl_setStyle()));

    QAction * localHostRunMode = new QAction( tr( "Local host" ), this );
    localHostRunMode->setCheckable( true );
    localHostRunMode->setChecked( LOCAL_HOST == runMode );
    runModeActions << localHostRunMode;
    connect( localHostRunMode, SIGNAL( triggered() ), SLOT( sl_setRunMode() ) );

    QAction * remoteMachineRunMode = new QAction( tr( "Remote machine" ), this );
    remoteMachineRunMode->setCheckable( true );
    remoteMachineRunMode->setChecked( REMOTE_MACHINE == runMode );
    runModeActions << remoteMachineRunMode;
    connect( remoteMachineRunMode, SIGNAL( triggered() ), SLOT( sl_setRunMode() ) );

    QAction * notShowScriptAction = new QAction( tr( "Hide scripting options" ), this );
    notShowScriptAction->setCheckable( true );
    scriptingActions << notShowScriptAction;
    notShowScriptAction->setChecked(!scriptingMode);
    connect( notShowScriptAction, SIGNAL( triggered() ), SLOT( sl_changeScriptMode() ) );

    QAction * showScriptAction = new QAction( tr( "Show scripting options" ), this );
    showScriptAction->setCheckable( true );
    showScriptAction->setChecked(scriptingMode);
    scriptingActions << showScriptAction;
    connect( showScriptAction, SIGNAL( triggered() ), SLOT( sl_changeScriptMode() ) );

    unlockAction = new QAction(tr("Unlock Scene"), this);
    unlockAction->setCheckable(true);
    unlockAction->setChecked(true);
    connect(unlockAction, SIGNAL(toggled(bool)), SLOT(sl_toggleLock(bool)));

    createScriptAcction = new QAction(tr("Create element with script..."), this);
    createScriptAcction->setIcon(QIcon(":workflow_designer/images/script.png"));
    connect(createScriptAcction, SIGNAL(triggered()), SLOT(sl_createScript()));

    editScriptAction = new QAction(tr("Edit script of the element..."),this);
    editScriptAction->setIcon(QIcon(":workflow_designer/images/script_edit.png"));
    editScriptAction->setEnabled(false); // because user need to select actor with script to enable it
    connect(editScriptAction, SIGNAL(triggered()), SLOT(sl_editScript()));

    externalToolAction = new QAction(tr("Create element with command line tool..."), this);
    externalToolAction->setIcon(QIcon(":workflow_designer/images/external_cmd_tool.png"));
    connect(externalToolAction, SIGNAL(triggered()), SLOT(sl_externalAction()));

    editExternalToolAction = new QAction(tr("Edit configuration..."),this);
    editExternalToolAction->setIcon(QIcon(":workflow_designer/images/external_cmd_tool.png"));
    editExternalToolAction->setEnabled(false); // because user need to select actor with script to enable it
    connect(editExternalToolAction, SIGNAL(triggered()), SLOT(sl_editExternalTool()));

    appendExternalTool = new QAction(tr("Add element with command line tool..."), this);
    appendExternalTool->setIcon(QIcon(":workflow_designer/images/external_cmd_tool_add.png"));
    connect(appendExternalTool, SIGNAL(triggered()), SLOT(sl_appendExternalToolWorker()));

    findPrototypeAction = new QAction(this);
    findPrototypeAction->setShortcut(QKeySequence::Find);
    connect(findPrototypeAction, SIGNAL(triggered()), SLOT(sl_findPrototype()));
    this->addAction(findPrototypeAction);
}

void WorkflowView::sl_findPrototype(){
    if(tabs->currentWidget() != palette){return;}

    palette->setFocus();

    static const int MIN_SIZE_FIND = 260;
    QList<int> sizes = splitter->sizes();
    int idx = splitter->indexOf(tabs);
    CHECK(idx >= 0 && idx < sizes.size(),);
    if(sizes.at(idx) < MIN_SIZE_FIND / 2){
        sizes[idx] = MIN_SIZE_FIND;
        splitter->setSizes(sizes);
    }    
}

void WorkflowView::sl_createScript() {
    CreateScriptElementDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) {
        QList<DataTypePtr > input = dlg.getInput();
        QList<DataTypePtr > output = dlg.getOutput();
        QList<Attribute*> attrs = dlg.getAttributes();
        QString name = dlg.getName();
        QString desc = dlg.getDescription();
        if(LocalWorkflow::ScriptWorkerFactory::init(input, output, attrs, name, desc, dlg.getActorFilePath())) {
            ActorPrototype *proto = WorkflowEnv::getProtoRegistry()->getProto(LocalWorkflow::ScriptWorkerFactory::ACTOR_ID + name);
            QRectF rect = scene->sceneRect();
            addProcess(createActor(proto, QVariantMap()), rect.center());
        }
    }
}

void WorkflowView::sl_externalAction() {
    CreateExternalProcessDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) {
        ExternalProcessConfig *cfg = dlg.config();
        if(LocalWorkflow::ExternalProcessWorkerFactory::init(cfg)) {
            ActorPrototype *proto = WorkflowEnv::getProtoRegistry()->getProto(cfg->name);
            QRectF rect = scene->sceneRect();
            addProcess(createActor(proto, QVariantMap()), rect.center());
        }
    }
}

void WorkflowView::sl_appendExternalToolWorker() {
    QString filter = DialogUtils::prepareFileFilter(WorkflowUtils::tr("UGENE workflow element"), QStringList() << "etc", true);
    QString url = QFileDialog::getOpenFileName(this, tr("Add element"), QString(), filter);
    if (!url.isEmpty()) {
        IOAdapter *io = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(GUrl(url)))->createIOAdapter();
        if(!io->open(url, IOAdapterMode_Read)) {
            coreLog.error(tr("Can't load element."));
            return;
        }
        QByteArray data;
        data.resize(MAX_FILE_SIZE);
        data.fill(0);
        io->readBlock(data.data(), MAX_FILE_SIZE);

        ExternalProcessConfig *cfg = HRSchemaSerializer::string2Actor(data.data());
        if(cfg) {
            if (WorkflowEnv::getProtoRegistry()->getProto(cfg->name)) {
                coreLog.error("Element with this name already exists");
            } else {
                cfg->filePath = url;
                LocalWorkflow::ExternalProcessWorkerFactory::init(cfg);
                ActorPrototype *proto = WorkflowEnv::getProtoRegistry()->getProto(cfg->name);
                QRectF rect = scene->sceneRect();
                addProcess(createActor(proto, QVariantMap()), rect.center());
            }
        } else {
            coreLog.error(tr("Can't load element."));
        }
        io->close();
    }
}

void WorkflowView::sl_editScript() {
    QList<Actor *> selectedActors = scene->getSelectedActors();
    if(selectedActors.size() == 1) {
        Actor *scriptActor = selectedActors.first();
        AttributeScript *script = scriptActor->getScript();
        if(script!= NULL) {
            ScriptEditorDialog scriptDlg(this,AttributeScriptDelegate::createScriptHeader(*script), script->getScriptText());
            if(scriptDlg.exec() == QDialog::Accepted) {
                script->setScriptText(scriptDlg.getScriptText());
                scriptActor->setScript(script);
            }
        }
    }
}

void WorkflowView::sl_editExternalTool() {
    QList<Actor *> selectedActors = scene->getSelectedActors();
    if (selectedActors.size() == 1) {
        ActorPrototype *proto = selectedActors.first()->getProto();

        ExternalProcessConfig *oldCfg = WorkflowEnv::getExternalCfgRegistry()->getConfigByName(proto->getId());
        ExternalProcessConfig *cfg = new ExternalProcessConfig(*oldCfg);
        CreateExternalProcessDialog dlg(this, cfg, true);
        if(dlg.exec() == QDialog::Accepted) {
            cfg = dlg.config();

            if (!(*oldCfg == *cfg)) {
                if (oldCfg->name != cfg->name) {
                    if (!QFile::remove(proto->getFilePath())) {
                        uiLog.error(tr("Can't remove element %1").arg(proto->getDisplayName()));
                    }
                }
                sl_protoDeleted(proto->getId());
                WorkflowEnv::getProtoRegistry()->unregisterProto(proto->getId());
                delete proto;

                LocalWorkflow::ExternalProcessWorkerFactory::init(cfg);
            }
            WorkflowEnv::getExternalCfgRegistry()->unregisterConfig(oldCfg->name);
            WorkflowEnv::getExternalCfgRegistry()->registerExternalTool(cfg);
            scene->sl_updateDocs();
        }
    }
}

void WorkflowView::sl_protoDeleted(const QString &id) {
    QList<WorkflowProcessItem*> deleteList;
    foreach(QGraphicsItem *i, scene->items()) {        
        if(i->type() == WorkflowProcessItemType) {
            WorkflowProcessItem *wItem = static_cast<WorkflowProcessItem *>(i);
            if(wItem->getProcess()->getProto()->getId() == id) {
                deleteList << wItem;
            }
        }
    }

    foreach(WorkflowProcessItem *item, deleteList) {
        removeProcessItem(item);
    }
    scene->update();
}

void WorkflowView::sl_protoListModified() {
    CHECK(NULL != elementsMenu, );
    palette->createMenu(elementsMenu);
}

void WorkflowView::addProcess(Actor *proc, const QPointF &pos) {
    schema->addProcess(proc);

    WorkflowProcessItem *it = new WorkflowProcessItem(proc);
    it->setPos(pos);
    scene->addItem(it);
    scene->setModified();

    ConfigurationEditor *editor = proc->getEditor();
    if (NULL != editor) {
        connect(editor, SIGNAL(si_configurationChanged()), scene, SIGNAL(configurationChanged()));
    }
    GrouperEditor *g = dynamic_cast<GrouperEditor*>(editor);
    MarkerEditor *m = dynamic_cast<MarkerEditor*>(editor);
    if (NULL != g || NULL != m) {
        connect(editor, SIGNAL(si_configurationChanged()), scene, SLOT(sl_refreshBindings()));
    }

    sl_procItemAdded();
    update();
}

void WorkflowView::removeProcessItem(WorkflowProcessItem *item) {
    CHECK(NULL != item, );
    Actor *actor = item->getProcess();
    scene->removeItem(item);
    delete item;

    scene->setModified();
    schema->removeProcess(actor);
    delete actor;

    removeWizards();
}

void WorkflowView::removeWizards() {
    qDeleteAll(schema->takeWizards());
    sl_updateUi();
}

void WorkflowView::removeBusItem(WorkflowBusItem *item) {
    Link *link = item->getBus();
    scene->removeItem(item);
    delete item;
    scene->setModified();
    onBusRemoved(link);
}

void WorkflowView::onBusRemoved(Link *link) {
    if (!sceneRecreation) {
        schema->removeFlow(link);
        schema->update();
    }
}

void WorkflowView::sl_toggleLock(bool b) {
    if (sender() != unlockAction) {
        unlockAction->setChecked(b);
        return;
    }

    if (b) {
        scene->setRunner(NULL);
    }

    deleteAction->setEnabled(b);
    selectAction->setEnabled(b);
    copyAction->setEnabled(b);
    pasteAction->setEnabled(b);
    cutAction->setEnabled(b);
    loadAction->setEnabled(b);

    iterationModeAction->setEnabled(b);
    configureIterationsAction->setEnabled(b);
    runAction->setEnabled(b);
    validateAction->setEnabled(b);
    configureParameterAliasesAction->setEnabled(b);
    configurePortAliasesAction->setEnabled(b);
    importSchemaToElement->setEnabled(b);

    propertyEditor->setEnabled(b);
    propertyEditor->setSpecialPanelEnabled(b);
    palette->setEnabled(b);

    scene->setLocked(!b);
    scene->update();
}

void WorkflowView::sl_setStyle() {
    StyleId s = qobject_cast<QAction* >(sender())->data().value<StyleId>();
    QList<QGraphicsItem*> lst = scene->selectedItems();
    if (lst.isEmpty()) {
        lst = scene->items();
    }
    foreach(QGraphicsItem* it, lst) {
        switch (it->type()) {
            case WorkflowProcessItemType:
            case WorkflowPortItemType:
            case WorkflowBusItemType:
            (static_cast<StyledItem*>(it))->setStyle(s);
        }
    }
    scene->update();
}

// FIXME: move to utils classes
static void removeUrlLocationParameter( Actor * actor ) {
    assert( NULL != actor );
    Attribute * attr = actor->getParameter( BaseAttributes::URL_LOCATION_ATTRIBUTE().getId() );
    if( NULL != attr ) {
        Attribute * removed = actor->removeParameter( BaseAttributes::URL_LOCATION_ATTRIBUTE().getId() );
        assert( attr == removed );
        Q_UNUSED(removed);

        delete attr;
        delete actor->getEditor()->removeDelegate( BaseAttributes::URL_LOCATION_ATTRIBUTE().getId() );
    }
}

// FIXME: move to utils classes
static void addUrlLocationParameter( Actor * actor ) {
    assert( NULL != actor );
    Attribute * urlAttr = actor->getParameter( BaseAttributes::URL_IN_ATTRIBUTE().getId() );
    Attribute * urlLocationAttr = actor->getParameter( BaseAttributes::URL_LOCATION_ATTRIBUTE().getId() );
    if( NULL != urlAttr && NULL == urlLocationAttr ) {
        actor->addParameter( BaseAttributes::URL_LOCATION_ATTRIBUTE().getId(), 
            new Attribute( BaseAttributes::URL_LOCATION_ATTRIBUTE(), BaseTypes::BOOL_TYPE(), false, true ) );
        SchemaRunModeDelegate * runModeDelegate = new SchemaRunModeDelegate();
        URLDelegate * urlDelegate = qobject_cast<URLDelegate*>
            ( actor->getEditor()->getDelegate( BaseAttributes::URL_IN_ATTRIBUTE().getId() ) );
        QObject::connect( runModeDelegate, SIGNAL( si_showOpenFileButton( bool ) ), urlDelegate, SLOT( sl_showEditorButton( bool ) ) );
        actor->getEditor()->addDelegate( runModeDelegate, BaseAttributes::URL_LOCATION_ATTRIBUTE().getId() );
    }
}

void WorkflowView::sl_setRunMode() {
    QAction * a = qobject_cast<QAction*>( sender() );
    CHECK(NULL != a, );
    if( runModeActions[0] == a ) { // local host run mode
        runMode = LOCAL_HOST;
    } else if( runModeActions[1] == a ) { // remote machine run mode
        runMode = REMOTE_MACHINE;
    }

    runModeActions[0]->setChecked( LOCAL_HOST == runMode );
    runModeActions[1]->setChecked( REMOTE_MACHINE == runMode );

    // change actors that are on pallette now
    foreach(Actor *actor, schema->getProcesses()) {
        switch( runMode ) {
        case LOCAL_HOST:
            removeUrlLocationParameter(actor);
            break;
        case REMOTE_MACHINE:
            addUrlLocationParameter(actor);
            break;
        default:
            assert(false);
        }
    }

    scene->sl_deselectAll();
}

void WorkflowView::sl_changeScriptMode() {
    QAction *a = qobject_cast<QAction*>(sender());
    if(a != NULL) {
        if(a == scriptingActions[0]){
            scriptingMode = false;
        } else if(a == scriptingActions[1]) {
            scriptingMode = true;
        }
    } // else invoked from constructor

    scriptingActions[0]->setChecked(!scriptingMode);
    scriptingActions[1]->setChecked(scriptingMode);
    propertyEditor->changeScriptMode(scriptingMode);
}

void WorkflowView::sl_toggleStyle() {
    foreach(QGraphicsItem* it, scene->selectedItems()) {
        StyleId s = (static_cast<StyledItem*>(it))->getStyle();
        if (s == ItemStyles::SIMPLE) {
            s = ItemStyles::EXTENDED;
        } else {
            s = ItemStyles::SIMPLE;
        }
        (static_cast<StyledItem*>(it))->setStyle(s);
    }
    scene->update();
}

void WorkflowView::sl_refreshActorDocs() {
    foreach(QGraphicsItem* it, scene->items()) {
        if (it->type() == WorkflowProcessItemType) {
            Actor* a = qgraphicsitem_cast<WorkflowProcessItem*>(it)->getProcess();
            a->getDescription()->update(propertyEditor->getCurrentIteration().getParameters(a->getId()));
        }
    }
}

void WorkflowView::setupMDIToolbar(QToolBar* tb) {
    tb->addAction(newAction);
    tb->addAction(loadAction);
    tb->addAction(saveAction);
    tb->addAction(saveAsAction);
    tb->addSeparator();
    tb->addAction(validateAction);
    tb->addAction(runAction);
    tb->addAction(stopAction);
    tb->addSeparator();
    tb->addAction(configureParameterAliasesAction);
    tb->addAction(configurePortAliasesAction);
    tb->addAction(importSchemaToElement);
    tb->addSeparator();
    tb->addAction(iterationModeAction);
    tb->addAction(configureIterationsAction);
    tb->addSeparator();
    tb->addAction(createScriptAcction);
    tb->addAction(editScriptAction);
    tb->addSeparator();
    tb->addAction(externalToolAction);
    tb->addAction(appendExternalTool);
    tb->addSeparator();
    tb->addAction(copyAction);
    tb->addAction(pasteAction);
    pasteAction->setEnabled(!lastPaste.isEmpty());
    tb->addAction(cutAction);
    tb->addAction(deleteAction);
    tb->addSeparator();
    tb->addWidget(sceneScaleCombo);
    tb->addSeparator();

    QToolButton* tt = new QToolButton(tb);
    QMenu* ttMenu = new QMenu(tr("Element style"), this);
    foreach(QAction* a, styleActions) {
        ttMenu->addAction(a);
    }
    tt->setDefaultAction(ttMenu->menuAction());
    tt->setPopupMode(QToolButton::InstantPopup);
    tb->addWidget(tt);
    
    QToolButton * runModeToolButton = new QToolButton(tb);
    QMenu * runModeMenu = new QMenu( tr( "Run mode" ), this );
    foreach( QAction * a, runModeActions ) {
        runModeMenu->addAction( a );
    }
    runModeToolButton->setDefaultAction( runModeMenu->menuAction() );
    runModeToolButton->setPopupMode( QToolButton::InstantPopup );
    tb->addWidget( runModeToolButton );

    QToolButton * scriptingMode = new QToolButton(tb);
    QMenu * scriptingModeMenu = new QMenu( tr( "Scripting mode" ), this );
    foreach( QAction * a, scriptingActions ) {
        scriptingModeMenu->addAction( a );
    }
    scriptingMode->setDefaultAction( scriptingModeMenu->menuAction() );
    scriptingMode->setPopupMode( QToolButton::InstantPopup );
    tb->addWidget( scriptingMode );
    tb->addAction(showWizard);
}

void WorkflowView::setupViewMenu(QMenu* m) {
    elementsMenu = palette->createMenu(tr("Add element"));
    m->addMenu(elementsMenu);
    m->addAction(copyAction);
    m->addAction(pasteAction);
    pasteAction->setEnabled(!lastPaste.isEmpty());
    m->addAction(cutAction);
    m->addAction(deleteAction);
    m->addAction(selectAction);
    m->addSeparator();
    m->addSeparator();
    m->addAction(newAction);
    m->addAction(loadAction);
    m->addAction(saveAction);
    m->addAction(saveAsAction);
    m->addAction(exportAction);
    m->addSeparator();
    m->addAction(validateAction);
    m->addAction(runAction);
    m->addAction(stopAction);
    m->addSeparator();
    m->addAction(configureParameterAliasesAction);
    m->addAction(configurePortAliasesAction);
    m->addAction(importSchemaToElement);
    m->addSeparator();
    m->addAction(iterationModeAction);
    m->addAction(configureIterationsAction);
    m->addSeparator();
    m->addAction(createScriptAcction);
    m->addAction(editScriptAction);
    m->addSeparator();
    m->addAction(externalToolAction);
    m->addAction(appendExternalTool);
    m->addSeparator();

    QMenu* ttMenu = new QMenu(tr("Element style"));
    foreach(QAction* a, styleActions) {
        ttMenu->addAction(a);
    }
    m->addMenu(ttMenu);
    
    QMenu * runModeMenu = new QMenu( tr( "Run mode" ) );
    foreach( QAction * a, runModeActions ) {
        runModeMenu->addAction( a );
    }
    m->addMenu( runModeMenu );

    QMenu* scriptMenu = new QMenu(tr("Scripting mode"));
    foreach(QAction* a, scriptingActions) {
        scriptMenu->addAction(a);
    }
    m->addMenu(scriptMenu);
    
    if (!unlockAction->isChecked()) {
        m->addSeparator();
        m->addAction(unlockAction);
    }
    m->addSeparator();
}

void WorkflowView::setupContextMenu(QMenu* m) {
    if (!unlockAction->isChecked()) {
        m->addAction(unlockAction);
        return;
    }

    if(!lastPaste.isEmpty()) {
        m->addAction(pasteAction);
    }
    QList<QGraphicsItem*> sel = scene->selectedItems();
    if (!sel.isEmpty()) {
        if(!((sel.size() == 1 && sel.first()->type() == WorkflowBusItemType) || sel.first()->type() == WorkflowPortItemType)) {
            m->addAction(copyAction);
            m->addAction(cutAction);
        }
        if(!(sel.size() == 1 && sel.first()->type() == WorkflowPortItemType)) {
            m->addAction(deleteAction);
        }
        m->addSeparator();
        if (sel.size() == 1 && sel.first()->type() == WorkflowProcessItemType) {
            WorkflowProcessItem* wit = qgraphicsitem_cast<WorkflowProcessItem*>(sel.first());
            Actor *scriptActor = wit->getProcess();
            AttributeScript *script = scriptActor->getScript();
            if(script) {
                m->addAction(editScriptAction);
            }

            ActorPrototype *p = scriptActor->getProto();
            if (p->isExternalTool()) {
                m->addAction(editExternalToolAction);
            }

            m->addSeparator();

            QMenu* itMenu = new QMenu(tr("Element properties"));
            foreach(QAction* a, wit->getContextMenuActions()) {
                itMenu->addAction(a);
            }
            m->addMenu(itMenu);
        }
        if(!(sel.size() == 1 && (sel.first()->type() == WorkflowBusItemType || sel.first()->type() == WorkflowPortItemType))) {
            QMenu* ttMenu = new QMenu(tr("Element style"));
            foreach(QAction* a, styleActions) {
                ttMenu->addAction(a);
            }
            m->addMenu(ttMenu);
        }
    }
    m->addSeparator();

    QMenu * runModeMenu = new QMenu( tr( "Run mode" ) );
    foreach( QAction * a, runModeActions ) {
        runModeMenu->addAction( a );
    }
    m->addMenu( runModeMenu );
    m->addSeparator();

    m->addAction(selectAction);
    m->addMenu(palette->createMenu(tr("Add element")));
}

void WorkflowView::sl_pickInfo(QListWidgetItem* info) {
    ActorId id = info->data(ACTOR_REF).value<ActorId>();
    foreach(QGraphicsItem* it, scene->items()) {
        if (it->type() == WorkflowProcessItemType)
        {
            WorkflowProcessItem* proc = static_cast<WorkflowProcessItem*>(it);
            if (proc->getProcess()->getId() != id) {
                continue;
            }
            scene->clearSelection();
            QString pid = info->data(PORT_REF).toString();
            WorkflowPortItem* port = proc->getPort(pid);
            if (port) {
                port->setSelected(true);
            } else {
                proc->setSelected(true);
                int itid = info->data(ITERATION_REF).toInt();
                propertyEditor->selectIteration(itid);
            }
            return;
        }
    }
}

bool WorkflowView::sl_validate(bool notify) {
    if(schema->getProcesses().isEmpty()) {
        QMessageBox::warning(this, tr("Empty schema!"), tr("Nothing to run: empty workflow schema"));
        return false;
    }
    
    propertyEditor->commit();
    infoList->clear();
    QList<QListWidgetItem*> lst;
    bool good = WorkflowUtils::validate(*schema, &lst);

    if (lst.count() != 0) {
        foreach(QListWidgetItem* wi, lst) {
            infoList->addItem(wi);
        }
        infoList->parentWidget()->show();
        QList<int> s = infoSplitter->sizes();
        if (s[s.size() - 2] == 0) {
            s[s.size() - 2] = qMin(infoList->sizeHint().height(), 300);
            infoSplitter->setSizes(s);
        }
    } else {
        infoList->parentWidget()->hide();
    }
    if (!good) {
        QMessageBox::warning(this, tr("Schema cannot be executed"), 
            tr("Please fix issues listed in the error list (located under schema)."));
    } else {
        if (notify) {
            QMessageBox::information(this, tr("Schema is valid"), 
                tr("Schema is valid.\nWell done!"));
        }
    }
    return good;
}

void WorkflowView::localHostLaunch() {
    if (!sl_validate(false)) {
        return;
    }
    if (schema->getDomain().isEmpty()) {
        // TODO: user choice
        schema->setDomain(WorkflowEnv::getDomainRegistry()->getAllIds().value(0));
    }

    const Schema *s = getSchema();
    WorkflowAbstractRunner * t = NULL;
    if(WorkflowSettings::runInSeparateProcess()) {
        t = new WorkflowRunInProcessTask(*s, s->getIterations());
    } else {
        t = new WorkflowRunTask(*s, s->getIterations());
    }

    t->setReportingEnabled(true);
    if (WorkflowSettings::monitorRun()) {
        unlockAction->setChecked(false);
        scene->setRunner(t);
        connect(t, SIGNAL(si_ticked()), scene, SLOT(update()));
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_toggleLock()));
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void WorkflowView::remoteLaunch() {
    if( !sl_validate(false) ) {
        return;
    }
    if (schema->getDomain().isEmpty() ) {
        schema->setDomain(WorkflowEnv::getDomainRegistry()->getAllIds().value(0));
    }

    RemoteMachineMonitor * rmm = AppContext::getRemoteMachineMonitor();
    assert( NULL != rmm );
    RemoteMachineSettingsPtr settings = RemoteMachineMonitorDialogController::selectRemoteMachine(rmm, true);
    if (settings == NULL) {
        return;
    }
    assert(settings->getMachineType() == RemoteMachineType_RemoteService);
    const Schema *s = getSchema();
    AppContext::getTaskScheduler()->registerTopLevelTask(new RemoteWorkflowRunTask(settings, *s, s->getIterations()));
}

void WorkflowView::sl_launch() {
    switch( runMode ) {
    case LOCAL_HOST:
        localHostLaunch();
        break;
    case REMOTE_MACHINE:
        remoteLaunch();
        break;
    default:
        assert( false );
    }
}

void WorkflowView::sl_stop() {
    Task *runningWorkflow = scene->getRunner();
    if (runningWorkflow) {
        runningWorkflow->cancel();
    }
}

void WorkflowView::sl_iterationsMode() {
    bool iterated = iterationModeAction->isChecked();

    if (!iterated && schema->getIterations().size() > 1) {
        QMessageBox askBox(QMessageBox::Question, tr("Workflow Designer"),
            tr("Do you really want to switch off iteration mode?"),
            QMessageBox::Yes | QMessageBox::No, this);
        askBox.setDefaultButton(QMessageBox::Yes);
        askBox.setTextFormat(Qt::RichText);
        askBox.setInformativeText(tr("A data from the current \"%1\" iteration will be saved "
            "and other iterations data will be lost.").arg(propertyEditor->getCurrentIteration().name));
        if (QMessageBox::No == askBox.exec()) {
            iterationModeAction->setChecked(true);
            return;
        }
    }

    if (scene->isIterated() != iterated) {
        setSceneIterated(iterated, propertyEditor->getCurrentIteration());
        propertyEditor->setIterated(iterated);
        propertyEditor->resetIterations();
        scene->setModified(true);
    }
}

void WorkflowView::sl_configureIterations() {
    propertyEditor->commit();
    SchemaConfigurationDialog d(*schema, schema->getIterations(), this);
    int ret = d.exec();
    if (d.hasModifications()) {
        schema->setIterations(d.getIterations());
        propertyEditor->resetIterations();
        scene->iterationsChanged();
    }
    if (QDialog::Accepted == ret) {
        sl_launch();
    }
}

void WorkflowView::sl_configureParameterAliases() {
    SchemaAliasesConfigurationDialogImpl dlg(*schema, this );
    int ret = QDialog::Accepted;
    do {
        ret = dlg.exec();
        if( ret == QDialog::Accepted ) {
            if(!dlg.validateModel()) {
                QMessageBox::critical( this, tr("Bad input!"), tr("Aliases for schema parameters should be different!") );
                continue;
            }
            // clear aliases before inserting new
            foreach (Actor * actor, schema->getProcesses()) {
                actor->getParamAliases().clear();
            }
            SchemaAliasesCfgDlgModel model = dlg.getModel();
            foreach(const ActorId & id, model.aliases.keys()) {
                foreach(const Descriptor & d, model.aliases.value(id).keys()) {
                    Actor * actor = schema->actorById(id);
                    assert(actor != NULL);
                    QString alias = model.aliases.value(id).value(d);
                    assert(!alias.isEmpty());
                    actor->getParamAliases().insert(d.getId(), alias);
                    QString help = model.help.value(id).value(d);
                    if( !help.isEmpty() ) {
                        actor->getAliasHelp().insert(alias, help);
                    }
                }
            }
            break;
        } else if( ret == QDialog::Rejected ) {
            break;
        } else {
            assert(false);
        }
    } while( ret == QDialog::Accepted );
}

void WorkflowView::sl_configurePortAliases() {
    PortAliasesConfigurationDialog dlg(*schema, this);
    if (QDialog::Accepted == dlg.exec()) {
        PortAliasesCfgDlgModel model = dlg.getModel();

        QList<PortAlias> portAliases;
        foreach (Port *port, model.ports.keys()) {
            PortAlias portAlias(port, model.ports.value(port).first, model.ports.value(port).second);

            foreach (Descriptor slotDescr, model.aliases.value(port).keys()) {
                QString actorId;
                QString slotId;
                {
                    if (port->isInput()) {
                        actorId = port->owner()->getId();
                        slotId = slotDescr.getId();
                    } else {
                        QStringList tokens = slotDescr.getId().split(':');
                        assert(2 == tokens.size());
                        actorId = tokens[0];
                        slotId = tokens[1];
                    }
                }

                Port *sourcePort = NULL;
                foreach (Port *p, schema->actorById(actorId)->getPorts()) {
                    DataTypePtr dt = p->Port::getType();
                    QList<Descriptor> descs = dt->getAllDescriptors();
                    if(descs.contains(slotId)) {
                        sourcePort = p;
                        break;
                    }
                }
                assert(NULL != sourcePort);

                portAlias.addSlot(sourcePort, slotId, model.aliases.value(port).value(slotDescr));
            }
            portAliases.append(portAlias);
        }

        schema->setPortAliases(portAliases);
    }
}

void WorkflowView::sl_importSchemaToElement() {
    QString error;
    if (!schema->getWizards().isEmpty()) {
        error = WorkflowView::tr("The schema contains a wizard. Sorry, but current version of "
            "UGENE doesn't support of wizards in the includes.");
        QMessageBox::critical(this, tr("Error"), error);
    } else if (WorkflowUtils::validateSchemaForIncluding(*schema, error)) {
        ImportSchemaDialog d(this);
        if (d.exec()) {
            Schema *s = new Schema();
            U2OpStatusImpl os;
            HRSchemaSerializer::deepCopy(*schema, s, os);
            SAFE_POINT_OP(os, );
            QString typeName = d.getTypeName();

            s->setTypeName(typeName);
            QString text = HRSchemaSerializer::schema2String(*s, NULL);

            QString path = WorkflowSettings::getIncludedElementsDirectory()
                + typeName + "." + WorkflowUtils::WD_FILE_EXTENSIONS.first();
            QFile file(path);
            file.open(QIODevice::WriteOnly);
            file.write(text.toLatin1());
            file.close();

            ActorPrototype *proto = IncludedProtoFactory::getSchemaActorProto(s, typeName, path);
            WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_INCLUDES(), proto);
            WorkflowEnv::getSchemaActorsRegistry()->registerSchema(typeName, s);
        }
    } else {
        QMessageBox::critical(this, tr("Error"), error);
    }
}

void WorkflowView::sl_selectPrototype(Workflow::ActorPrototype* p) {
    propertyEditor->setEditable(!p);
    scene->clearSelection();
    currentProto = p;

    propertyEditor->reset();
    if (!p) {
        scene->views().at(0)->unsetCursor();
        propertyEditor->changeScriptMode(scriptingMode);
    } else {
        delete currentActor;
        currentActor = createActor(p, QVariantMap());
        propertyEditor->setDescriptor(p, tr("Drag the palette element to the scene or just click on the scene to add the element."));
        scene->views().at(0)->setCursor(Qt::CrossCursor);
    }
}

QList<Iteration> WorkflowView::getIterations(const QList<Actor*> &actors) const {
    QList<Iteration> result;
    foreach (const Iteration &it, schema->getIterations()) {
        Iteration copy(it.name);
        foreach (Actor *a, actors) {
            if (it.cfg.contains(a->getId())) {
                copy.cfg.insert(a->getId(), it.cfg[a->getId()]);
            }
        }
        if (!copy.cfg.isEmpty()) {
            result << copy;
        }
    }
    return result;
}

void WorkflowView::sl_copyItems() {
    QList<WorkflowProcessItem*> procs;
    foreach(QGraphicsItem* item, scene->selectedItems()) {
        if (item->type() == WorkflowProcessItemType) {
            procs << qgraphicsitem_cast<WorkflowProcessItem*>(item);
        }
    }
    if (procs.isEmpty()) {
        return;
    }

    QList<Actor*> actors = scene->getSelectedActors();
    Metadata actorMeta = getMeta(procs);
    lastPaste = HRSchemaSerializer::items2String(actors, getIterations(actors), &actorMeta);
    pasteAction->setEnabled(true);
    QApplication::clipboard()->setText(lastPaste);
    pasteCount = 0;
}

void WorkflowView::sl_cutItems() {
    sl_copyItems();
    scene->sl_deleteItem();
}

void WorkflowView::sl_pasteSample(const QString& s) {
    tabs->setCurrentIndex(ElementsTab);
    if (scene->items().isEmpty()) {
        // fixing bug with pasting same schema 2 times
        {
            lastPaste.clear();
        }
        sl_pasteItems(s, true);
        sl_setRunMode();
        sl_updateTitle();
        scene->setIterated(false);
        sl_updateUi();
        scene->connectConfigurationEditors();
        scene->sl_selectAll();
        foreach(QGraphicsItem* it, scene->selectedItems()){
            StyledItem* styledItem = static_cast<StyledItem*>(it);
            if (styledItem) {
                styledItem->setStyle(WorkflowSettings::defaultStyle());
            }
        }
        scene->sl_deselectAll();
        scene->update();
    } else {
        scene->clearScene();
        propertyEditor->resetIterations();
        schema->reset();
        sl_pasteSample(s);
    }
}

static QMap<ActorId, ActorId> getUniquePastedActorIds(const QList<Actor*> &pasted, const QList<Actor*> &origin) {
    QMap<ActorId, ActorId> result;
    QStringList uniqueIds;
    foreach (Actor *a, origin) {
        uniqueIds << aid2str(a->getId());
    }
    foreach (Actor *a, pasted) {
        QString uniqId = WorkflowUtils::createUniqueString(aid2str(a->getId()), "-", uniqueIds);
        uniqueIds << uniqId;
        ActorId newId = str2aid(uniqId);
        if (newId != a->getId()) {
            result[a->getId()] = newId;
        }
    }
    return result;
}

static void renamePastedSchemaActors(Schema &pasted, Metadata &meta, Schema *origin) {
    QMap<ActorId, ActorId> mapping = getUniquePastedActorIds(pasted.getProcesses(), origin->getProcesses());
    foreach (const ActorId &id, mapping.keys()) {
        pasted.renameProcess(id, mapping[id]);
    }
    meta.renameActors(mapping);
}

void WorkflowView::sl_pasteItems(const QString &s, bool updateSchemaInfo) {
    QString tmp = s.isNull() ? QApplication::clipboard()->text() : s;
    if (tmp == lastPaste) {
        ++pasteCount;
    } else {
        pasteCount = 0;
        lastPaste = tmp;
    }
    QByteArray lpt = lastPaste.toLatin1();
    DocumentFormat* wf = AppContext::getDocumentFormatRegistry()->getFormatById(WorkflowDocFormat::FORMAT_ID);
    if (wf->checkRawData(lpt).score != FormatDetection_Matched) {
        return;
    }
    disconnect(scene, SIGNAL(selectionChanged()), this, SLOT(sl_editItem()));
    scene->clearSelection();
    connect(scene, SIGNAL(selectionChanged()), SLOT(sl_editItem()));

    Schema pastedS;
    pastedS.setDeepCopyFlag(true);
    Metadata pastedM;
    QString msg = HRSchemaSerializer::string2Schema(lastPaste, &pastedS, &pastedM);
    if (!msg.isEmpty()) {
        uiLog.error("Paste issues: " + msg);
        return;
    }
    renamePastedSchemaActors(pastedS, pastedM, schema);
    if (schema->getProcesses().isEmpty()) {
        schema->setWizards(pastedS.takeWizards());
    }
    schema->merge(pastedS);
    updateMeta();
    meta.mergeVisual(pastedM);
    if (updateSchemaInfo) {
        meta.name = pastedM.name;
        meta.comment = pastedM.comment;
    }
    pastedS.setDeepCopyFlag(false);
    recreateScene();
    scene->connectConfigurationEditors();
    propertyEditor->resetIterations();

    foreach (QGraphicsItem *it, scene->items()) {
        WorkflowProcessItem *proc = qgraphicsitem_cast<WorkflowProcessItem*>(it);
        if (NULL != proc) {
            if (NULL != pastedS.actorById(proc->getProcess()->getId())) {
                it->setSelected(true);
            }
        }
    }

    int shift = GRID_STEP*(pasteCount);
    foreach(QGraphicsItem * it, scene->selectedItems()) {
        it->moveBy(shift, shift);
    }
}

void WorkflowView::recreateScene() {
    sceneRecreation = true;
    SceneCreator sc(schema, meta);
    sc.recreateScene(scene);
    sceneRecreation = false;
}

void WorkflowView::sl_procItemAdded() {
    currentActor = NULL;
    propertyEditor->setEditable(true);
    scene->invalidate(QRectF(), QGraphicsScene::BackgroundLayer);
    if (!currentProto) {
        return;
    }

    uiLog.trace(currentProto->getDisplayName() + " added");
    palette->resetSelection();
    currentProto = NULL;
    assert(scene->views().size() == 1);
    scene->views().at(0)->unsetCursor();
}

void WorkflowView::sl_showEditor() {
    propertyEditor->show();
    QList<int> s = splitter->sizes();
    if (s.last() == 0) {
        s.last() = propertyEditor->sizeHint().width();
        splitter->setSizes(s);
    }
}

void WorkflowView::sl_editItem() {
    QList<QGraphicsItem*> list = scene->selectedItems();
    if (list.size() == 1) {
        QGraphicsItem* it = list.at(0);
        if (it->type() == WorkflowProcessItemType) {
            Actor *a = qgraphicsitem_cast<WorkflowProcessItem*>(it)->getProcess();
            propertyEditor->editActor(a);
            return;
        }
        Port* p = NULL;
        if (it->type() == WorkflowBusItemType) {
            p = qgraphicsitem_cast<WorkflowBusItem*>(it)->getInPort()->getPort();
        } else if (it->type() == WorkflowPortItemType) {
            p = qgraphicsitem_cast<WorkflowPortItem*>(it)->getPort();
        }
        if (p) {
            if (qobject_cast<IntegralBusPort*>(p)) 
            {
                BusPortEditor* ed = new BusPortEditor(qobject_cast<IntegralBusPort*>(p));
                ed->setParent(p);
                p->setEditor(ed);
            }
        }
        propertyEditor->editPort(p);
    } else {
        propertyEditor->reset();
    }
}

void WorkflowView::sl_onSelectionChanged() {
    QList<Actor*> actorsSelected = scene->getSelectedActors();
    editScriptAction->setEnabled(actorsSelected.size() == 1 && actorsSelected.first()->getScript() != NULL);
    editExternalToolAction->setEnabled(actorsSelected.size() == 1 && actorsSelected.first()->getProto()->isExternalTool());
}

void WorkflowView::sl_exportScene() {
    propertyEditor->commit();

    ExportImageDialog dialog(sceneView->viewport(),sceneView->viewport()->rect(),true,true);
    dialog.exec();
}

void WorkflowView::sl_saveScene() {
    if (meta.url.isEmpty()) {
        WorkflowMetaDialog md(this, meta);
        int rc = md.exec();
        if (rc != QDialog::Accepted) {
            return;
        }
        meta = md.meta;
        sl_updateTitle();
    }
    propertyEditor->commit();
    Task* t = new SaveWorkflowSceneTask(getSchema(), getMeta()); 
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_onSceneSaved()));
}

void WorkflowView::sl_saveSceneAs() {
    WorkflowMetaDialog md(this, meta);
    int rc = md.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    propertyEditor->commit();
    meta = md.meta;
    Task* t = new SaveWorkflowSceneTask(getSchema(), getMeta());
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
    sl_updateTitle();
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_onSceneSaved()));
}

void WorkflowView::sl_showWizard() {
    if (schema->getWizards().size() > 0) {
        Wizard *w = schema->getWizards().first();
        WizardController controller(schema, w);
        QWizard *gui = controller.createGui();
        if (gui->exec() && !controller.isBroken()) {
            updateMeta();
            WizardController::ApplyResult res = controller.applyChanges(meta);
            if (WizardController::ACTORS_REPLACED == res) {
                recreateScene();
                schema->setWizards(QList<Wizard*>());
            }
            propertyEditor->resetIterations();
            scene->sl_updateDocs();
            scene->setModified();
        }
    }
}

void WorkflowView::sl_loadScene() {
    if (!confirmModified()) {
        return;
    }

    QString dir = AppContext::getSettings()->getValue(LAST_DIR, QString("")).toString();
    QString filter = DesignerUtils::getSchemaFileFilter(true, true);
    QString url = QFileDialog::getOpenFileName(0, tr("Open workflow schema file"), dir, filter);
    if (!url.isEmpty()) {
        AppContext::getSettings()->setValue(LAST_DIR, QFileInfo(url).absoluteDir().absolutePath());
        Task* t = new LoadWorkflowSceneTask(schema, &meta, scene, url); //FIXME unsynchronized meta usage
        TaskSignalMapper* m = new TaskSignalMapper(t);
        connect(m, SIGNAL(si_taskFinished(Task*)), SLOT(sl_onSceneLoaded()));
        if(LoadWorkflowTask::detectFormat(IOAdapterUtils::readFileHeader(url)) == LoadWorkflowTask::XML) {
            connect(m, SIGNAL(si_taskFinished(Task*)), SLOT(sl_xmlSchemaLoaded(Task*)));
        }
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
}

void WorkflowView::sl_xmlSchemaLoaded(Task* t) {
    assert(t != NULL);
    if(!t->hasError()) {
        QMessageBox::warning(this, tr("Warning!"), XML_SCHEMA_WARNING);
    } else {
        QMessageBox::warning(this, tr("Warning!"), XML_SCHEMA_APOLOGIZE);
    }
}

void WorkflowView::sl_newScene() {
    if (!confirmModified()) {
        return;
    }
    infoList->parentWidget()->hide();
    scene->sl_reset();
    meta.reset();
    meta.name = tr("New schema");
    schema->reset();
    sl_updateTitle();
    propertyEditor->resetIterations();
    scene->setModified(false);
    scene->update();
    this->sl_updateUi();
}

void WorkflowView::sl_onSceneLoaded() {
    sl_updateTitle();
    sl_updateUi();
    scene->centerView();
    propertyEditor->resetIterations();

    scene->setModified(false);
}

void WorkflowView::sl_onSceneSaved() {
    Task *t = dynamic_cast<Task*>(sender());
    CHECK(NULL != t, );
    if (t->isFinished() && !t->hasError()) {
        scene->setModified(false);
    }
}

void WorkflowView::sl_updateTitle() {
    setWindowTitle(tr("Workflow Designer - %1").arg(meta.name));    
}

void WorkflowView::sl_updateUi() {
    bool iterated = scene->isIterated();
    propertyEditor->setIterated(iterated);
    iterationModeAction->setChecked(iterated);
    scene->setModified(false);
    showWizard->setDisabled(schema->getWizards().isEmpty());
}

void WorkflowView::saveState() {
    AppContext::getSettings()->setValue(SPLITTER_STATE, splitter->saveState());
    AppContext::getSettings()->setValue(EDITOR_STATE, propertyEditor->saveState());
    AppContext::getSettings()->setValue(PALETTE_STATE, palette->saveState());
    AppContext::getSettings()->setValue(TABS_STATE, tabs->currentIndex());
}

bool WorkflowView::onCloseEvent() {
    saveState();
    if (!confirmModified()) {
        return false;
    }
    if (go) {
        go->setView(NULL);
    }
    return true;
}

bool WorkflowView::confirmModified() {
    propertyEditor->commit();
    if (scene->isModified() && !scene->items().isEmpty()) {
        AppContext::getMainWindow()->getMDIManager()->activateWindow(this);
        int ret = QMessageBox::question(this, tr("Workflow Designer"),
            tr("The schema has been modified.\n"
            "Do you want to save changes?"),
            QMessageBox::Save | QMessageBox::Discard
            | QMessageBox::Cancel,
            QMessageBox::Save);
        if (QMessageBox::Cancel == ret) {
            return false;
        } else if (QMessageBox::Discard != ret) {
            sl_saveScene();
        }
    }
    return true;
}

static QString newActorLabel(ActorPrototype *proto, const QList<Actor*> &procs) {
    QStringList allLabels;
    foreach(Actor *actor, procs) {
        allLabels << actor->getLabel();
    }
    return WorkflowUtils::createUniqueString(proto->getDisplayName(), " ", allLabels);
}

Actor * WorkflowView::createActor(ActorPrototype *proto, const QVariantMap &params) const {
    assert(NULL != proto);
    QString pId = proto->getId().replace(QRegExp("\\s"), "-");
    ActorId id = Schema::uniqueActorId(pId, schema->getProcesses());
    Actor *actor = proto->createInstance(id, NULL, params);
    assert(NULL != actor);

    actor->setLabel(newActorLabel(proto, schema->getProcesses()));
    if( WorkflowView::REMOTE_MACHINE == runMode ) {
        addUrlLocationParameter(actor);
    }
    return actor;
}

QList<Iteration> WorkflowView::getIterations() const {
    if (scene->isIterated()) {
        return schema->getIterations();
    } else {
        QList<Iteration> result;
        SAFE_POINT(0 == schema->getIterations().size(), "Non-iterated schema has iterations", result << Iteration());
        result << schema->extractIterationFromConfig();
        return result;
    }
}

void WorkflowView::setIterations(const QList<Iteration> &iters) {
    if (scene->isIterated()) {
        schema->setIterations(iters);
    } else {
        SAFE_POINT(1 == iters.size(), "More than one iteration are applied", );
        schema->applyIteration(iters.first());
    }
    scene->iterationsChanged();
}

WorkflowBusItem * WorkflowView::tryBind(WorkflowPortItem *from, WorkflowPortItem *to) {
    WorkflowBusItem* dit = NULL;

    if (from->getPort()->canBind(to->getPort())) {
        Port *src = from->getPort();
        Port *dest = to->getPort();
        if (src->isInput()) {
            src = to->getPort();
            dest = from->getPort();
        }
        if (WorkflowUtils::isPathExist(src, dest)) {
            return NULL;
        }

        Link *link = new Link(src, dest);
        schema->addFlow(link);
        dit = scene->addFlow(from, to, link);
    }
    return dit;
}

void WorkflowView::sl_updateSchema() {
    schema->update();
}

Workflow::Schema * WorkflowView::getSchema() const {
    return schema;
}

const Workflow::Metadata & WorkflowView::getMeta() {
    return updateMeta();
}

const Workflow::Metadata & WorkflowView::updateMeta() {
    meta.resetVisual();
    foreach (QGraphicsItem *it, scene->items()) {
        switch (it->type()) {
        case WorkflowProcessItemType:
            {
                WorkflowProcessItem *proc = qgraphicsitem_cast<WorkflowProcessItem*>(it);
                ActorVisualData visual(proc->getProcess()->getId());
                visual.setPos(proc->pos());
                ItemViewStyle *style = proc->getStyleById(proc->getStyle());
                if (NULL != style) {
                    visual.setStyle(style->getId());
                    if (style->getBgColor() != style->defaultColor()) {
                        visual.setColor(style->getBgColor());
                    }
                    if (style->defaultFont() != QFont()) {
                        visual.setFont(style->defaultFont());
                    }
                    if (ItemStyles::EXTENDED == style->getId()) {
                        ExtendedProcStyle *eStyle = dynamic_cast<ExtendedProcStyle*>(style);
                        if (!eStyle->isAutoResized()) {
                            visual.setRect(eStyle->boundingRect());
                        }
                    }
                }
                foreach (WorkflowPortItem *port, proc->getPortItems()) {
                    visual.setPortAngle(port->getPort()->getId(), port->getOrientarion());
                }
                meta.setActorVisualData(visual);
            }
            break;
        case WorkflowBusItemType:
            {
                WorkflowBusItem *bus = qgraphicsitem_cast<WorkflowBusItem*>(it);
                Port *src = bus->getBus()->source();
                Port *dst = bus->getBus()->destination();
                QPointF p = bus->getText()->pos();
                meta.setTextPos(src->owner()->getId(), src->getId(), dst->owner()->getId(), dst->getId(), p);
            }
            break;
        }
    }
    return meta;
}

Workflow::Metadata WorkflowView::getMeta(const QList<WorkflowProcessItem*> &items) {
    const Workflow::Metadata &meta = getMeta();
    Workflow::Metadata result;
    result.name = meta.name;
    result.url = meta.url;
    result.comment = meta.comment;

    foreach (WorkflowProcessItem *proc, items) {
        bool contains = false;
        ActorVisualData visual = meta.getActorVisualData(proc->getProcess()->getId(), contains);
        assert(contains);
        result.setActorVisualData(visual);
        foreach (WorkflowPortItem *port1, proc->getPortItems()) {
            foreach (WorkflowBusItem *bus, port1->getDataFlows()) {
                WorkflowPortItem *port2 = (bus->getInPort() == port1) ? bus->getOutPort() : bus->getInPort();
                WorkflowProcessItem *proc2 = port2->getOwner();
                if (!items.contains(proc2)) {
                    continue;
                }
                Port *src = bus->getBus()->source();
                Port *dst = bus->getBus()->destination();
                QPointF p = meta.getTextPos(src->owner()->getId(), src->getId(), dst->owner()->getId(), dst->getId(), contains);
                if (contains) {
                    result.setTextPos(src->owner()->getId(), src->getId(), dst->owner()->getId(), dst->getId(), p);
                }
            }
        }
    }
    return result;
}

void WorkflowView::setSceneIterated(bool iterated, const Iteration &defaultIteration) {
    scene->setIterated(iterated);
    if (!iterated) {
        schema->applyIteration(defaultIteration);
    } else {
        SAFE_POINT(0 == schema->getIterations().size(), "Non-iterated schema has iterations", );

        schema->addIteration(schema->extractIterationFromConfig());

        foreach (Actor *proc, schema->getProcesses()) {
            foreach (Attribute *a, proc->getAttributes()) {
                if (a->getGroup() != COMMON_GROUP) {
                    continue;
                }
                if (!a->isDefaultValue()) {
                    a->setAttributeValue(a->getDefaultPureValue());
                }
            }
        }
    }
}

/********************************
 * WorkflowScene
 ********************************/
static bool canDrop(const QMimeData* m, QList<ActorPrototype*>& lst) {
    if (m->hasFormat(WorkflowPalette::MIME_TYPE)) {
        QString id(m->data(WorkflowPalette::MIME_TYPE));
        ActorPrototype* proto = WorkflowEnv::getProtoRegistry()->getProto(id);
        if (proto) {
            lst << proto;
        }
    } else {
        foreach(QList<ActorPrototype*> l, WorkflowEnv::getProtoRegistry()->getProtos().values()) {
            foreach(ActorPrototype* proto, l) {
                if (proto->isAcceptableDrop(m)) {
                    lst << proto;
                }
            }
        }
    }
    return !lst.isEmpty();
}

WorkflowScene::WorkflowScene(WorkflowView *parent)
: QGraphicsScene(parent), controller(parent), modified(false), locked(false), runner(NULL), hint(0), iterated(false) {
    openDocumentsAction = new QAction(tr("Open document(s)"), this);
    connect(openDocumentsAction, SIGNAL(triggered()), SLOT(sl_openDocuments()));
}

WorkflowScene::~WorkflowScene() {
    sl_reset();
}

void WorkflowScene::sl_deleteItem() {
    assert(!locked);
    QList<WorkflowProcessItem*> items;
    foreach(QGraphicsItem* it, selectedItems()) {
        WorkflowProcessItem *proc = qgraphicsitem_cast<WorkflowProcessItem*>(it);
        WorkflowBusItem *bus = qgraphicsitem_cast<WorkflowBusItem*>(it);
        switch (it->type()) {
            case WorkflowProcessItemType:
                items << proc;
                break;
            case WorkflowBusItemType:
                controller->removeBusItem(bus);
                break;
        }
    }
    foreach(WorkflowProcessItem *it, items) {
        controller->removeProcessItem(it);
    }
    controller->propertyEditor->resetIterations();

    controller->update();
    emit configurationChanged();
    update();
}

QList<Actor*> WorkflowScene::getSelectedActors() const {
    QList<Actor*> list;
    foreach (QGraphicsItem *item, selectedItems()) {
        if (item->type() == WorkflowProcessItemType) {
            list << static_cast<WorkflowProcessItem*>(item)->getProcess();
        }
    }
    return list;
}

void WorkflowScene::contextMenuEvent(QGraphicsSceneContextMenuEvent * e) {
    QGraphicsScene::contextMenuEvent(e);
    if (!e->isAccepted()) {
        QMenu menu;
        controller->setupContextMenu(&menu);
        e->accept();
        menu.exec(e->screenPos());
    }
}

void WorkflowScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent) {
    if (!mouseEvent->isAccepted() && (mouseEvent->button() == Qt::LeftButton) && !selectedItems().isEmpty()) {
        emit processDblClicked();
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void WorkflowScene::dragEnterEvent(QGraphicsSceneDragDropEvent * event) {
    QList<ActorPrototype*> lst;
    if (!locked && canDrop(event->mimeData(), lst)) {
        event->acceptProposedAction();
    } else {
        QGraphicsScene::dragEnterEvent(event);
    }
}

void WorkflowScene::dragMoveEvent(QGraphicsSceneDragDropEvent * event) {
    QList<ActorPrototype*> lst;
    if (!locked && canDrop(event->mimeData(), lst)) {
        event->acceptProposedAction();
    } else {
        QGraphicsScene::dragMoveEvent(event);
    }
}

void WorkflowScene::dropEvent(QGraphicsSceneDragDropEvent * event) {
    QList<ActorPrototype*> lst;
    if (!locked && canDrop(event->mimeData(), lst))
    {
        QList<QGraphicsItem *> targets = items(event->scenePos());
        bool done = false;
        foreach(QGraphicsItem* it, targets) {
            WorkflowProcessItem* target = qgraphicsitem_cast<WorkflowProcessItem*>(it);
            if (target && lst.contains(target->getProcess()->getProto())) {
                clearSelection();
                QVariantMap params;
                Actor* a = target->getProcess();
                a->getProto()->isAcceptableDrop(event->mimeData(), &params);
                QMapIterator<QString, QVariant> cfg(params);
                while (cfg.hasNext())
                {
                    cfg.next();
                    a->setParameter(cfg.key(),cfg.value());
                }
                target->setSelected(true);
                done = true;
                break;
            }
        }
        if (!done) {
            ActorPrototype* proto = lst.size() > 1 ? ChooseItemDialog(controller).select(lst) : lst.first();
            if (proto) {
                Actor* a = controller->getActor();
                if (a) {
                    controller->addProcess( a, event->scenePos());
                }
                event->setDropAction(Qt::CopyAction);
            }
        }
    } 
    QGraphicsScene::dropEvent(event);
}

void WorkflowScene::clearScene() {
    sl_reset();
}

void WorkflowScene::setupLinkCtxMenu(const QString& href, Actor* actor, const QPoint& pos) {
    const QString& attributeId = WorkflowUtils::getParamIdFromHref(href);
    bool isInput = attributeId == BaseAttributes::URL_IN_ATTRIBUTE().getId();
    bool isOutput = attributeId == BaseAttributes::URL_OUT_ATTRIBUTE().getId();
    if (isInput || isOutput) {
        const ActorId& actorId = actor->getId();
        const Iteration& iteration = controller->propertyEditor->getCurrentIteration();
        const QVariantMap& cfg = iteration.getParameters(actorId);
        QString urlStr;
        if (cfg.keys().contains(attributeId)) {
            urlStr = cfg.value(attributeId).toString();
        } else {
            Attribute* attribute = actor->getParameter(attributeId);
            urlStr = attribute->getAttributePureValue().toString();
        }

        if (!urlStr.isEmpty()) {
            QMenu menu;
            openDocumentsAction->setData(urlStr);
            menu.addAction(openDocumentsAction);
            menu.exec(pos);
        }
    }
}

bool WorkflowScene::isIterated() const {
    return iterated;
}

void WorkflowScene::setIterated(bool value) {
    iterated = value;
}

void WorkflowScene::sl_openDocuments() {
    const QString& urlStr = qVariantValue<QString>(openDocumentsAction->data());
    const QStringList& _urls = WorkflowUtils::expandToUrls(urlStr);
    QList<GUrl> urls;
    foreach(const QString& url, _urls) {
        urls.append(url);
    }
    Task* t = AppContext::getProjectLoader()->openWithProjectTask(urls);
    if (t) {
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    } else {
        QMessageBox::critical(controller, tr("Workflow Designer"), tr("Unable to open specified documents. Watch log for details."));
    }
}

void WorkflowScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (!locked && !mouseEvent->isAccepted() && controller->selectedProto() && (mouseEvent->button() == Qt::LeftButton)) {
        controller->addProcess(controller->getActor(), mouseEvent->scenePos());
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void WorkflowScene::sl_selectAll() {
    foreach (QGraphicsItem *it, items()) {
        it->setSelected(true);
    }
}

void WorkflowScene::sl_deselectAll() {
    foreach (QGraphicsItem *it, items()) {
        it->setSelected(false);
    }
}

void WorkflowScene::sl_reset() {
    QList<QGraphicsItem*> list;
    QList<QGraphicsItem*> itemss = items();
    foreach (QGraphicsItem *it, itemss) {
        if (it->type() == WorkflowProcessItemType) {
            list << it;
        }
    }
    modified = false;
    foreach(QGraphicsItem* it, list) {
        removeItem(it);
        delete it;
    }
    iterated = false;
}

void WorkflowScene::setModified(bool b) {
    modified = b;
}

void WorkflowScene::setModified() {
    setModified(true);
}

void WorkflowScene::drawBackground(QPainter * painter, const QRectF & rect)
{
    if (WorkflowSettings::showGrid()) {
        int step = GRID_STEP;
        painter->setPen(QPen(QColor(200, 200, 255, 125)));
        // draw horizontal grid
        qreal start = round(rect.top(), step);
        if (start > rect.top()) {
            start -= step;
        }
        for (qreal y = start - step; y < rect.bottom(); ) {
            y += step;
            painter->drawLine(rect.left(), y, rect.right(), y);
        }
        // now draw vertical grid
        start = round(rect.left(), step);
        if (start > rect.left()) {
            start -= step;
        }
        for (qreal x = start - step; x < rect.right(); ) {
            x += step;
            painter->drawLine(x, rect.top(), x, rect.bottom());
        }
    }

    if (items().size() == 0) {
        // draw a hint on empty scene
        painter->setPen(Qt::darkGray);
        QFont f = painter->font();
        if (hint == SamplesTab) {
        } else {
            QTransform trans = painter->combinedTransform();
            f.setFamily("Courier New");
            f.setPointSizeF(f.pointSizeF()* 2./trans.m11());
            painter->setFont(f);
            QRectF res;
            painter->drawText(sceneRect(), Qt::AlignCenter, tr("Drop an element from the palette here"), &res);
            QPixmap pix(":workflow_designer/images/leftarrow.png");
            QPointF pos(res.left(), res.center().y());
            pos.rx() -= pix.width() + GRID_STEP;
            pos.ry() -= pix.height()/2;
            painter->drawPixmap(pos, pix);
        }
    }
}

void WorkflowScene::iterationsChanged() {
    assert(!locked);
    modified = true; 
    emit configurationChanged();
}

void WorkflowScene::centerView() {
    QRectF childRect;
    foreach (QGraphicsItem *child, items()) {
        QPointF childPos = child->pos();
        QTransform matrix = child->transform() * QTransform().translate(childPos.x(), childPos.y());
        childRect |= matrix.mapRect(child->boundingRect() | child->childrenBoundingRect());
    }
    update();
}

WorkflowBusItem * WorkflowScene::addFlow(WorkflowPortItem *from, WorkflowPortItem *to, Link *link) {
    WorkflowBusItem *dit = new WorkflowBusItem(from, to, link);
    from->addDataFlow(dit);
    to->addDataFlow(dit);

    addItem(dit);
    dit->updatePos();
    setModified(true);
    return dit;
}

void WorkflowScene::connectConfigurationEditors() {
    foreach(QGraphicsItem *i, items()) {        
        if(i->type() == WorkflowProcessItemType) {
            Actor *proc = static_cast<WorkflowProcessItem *>(i)->getProcess();
            ConfigurationEditor *editor = proc->getEditor();
            if (NULL != editor) {
                connect(editor, SIGNAL(si_configurationChanged()), this, SIGNAL(configurationChanged()));
            }
            GrouperEditor *g = dynamic_cast<GrouperEditor*>(editor);
            MarkerEditor *m = dynamic_cast<MarkerEditor*>(editor);
            if (NULL != g || NULL != m) {
                connect(editor, SIGNAL(si_configurationChanged()), controller, SLOT(sl_updateSchema()));
            }
        }
    }
}

/************************************************************************/
/* SceneCreator */
/************************************************************************/
SceneCreator::SceneCreator(Schema *_schema, const Workflow::Metadata &_meta)
: schema(_schema), meta(_meta), scene(NULL)
{

}

SceneCreator::~SceneCreator() {
    delete scene;
}

WorkflowScene * SceneCreator::recreateScene(WorkflowScene *_scene) {
    scene = _scene;
    scene->sl_reset();
    return createScene();
}

WorkflowScene * SceneCreator::createScene(WorkflowView *controller) {
    scene = new WorkflowScene(controller);
    scene->setSceneRect(QRectF(-3*WS, -3*WS, 5*WS, 5*WS));
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    return createScene();
    scene->setObjectName("scene");
}

WorkflowScene * SceneCreator::createScene() {
    QMap<Port*, WorkflowPortItem*> ports;
    foreach (Actor *actor, schema->getProcesses()) {
        WorkflowProcessItem *procItem = createProcess(actor);
        scene->addItem(procItem);
        foreach (WorkflowPortItem *portItem, procItem->getPortItems()) {
            ports[portItem->getPort()] = portItem;
        }
    }

    foreach (Link *link, schema->getFlows()) {
        createBus(ports, link);
    }

    bool iterated = (schema->getIterations().size() > 0);
    scene->setIterated(iterated);

    WorkflowScene *result = scene;
    scene = NULL;
    return result;
}

WorkflowProcessItem * SceneCreator::createProcess(Actor *actor) {
    WorkflowProcessItem *procItem = new WorkflowProcessItem(actor);
    bool contains = false;
    ActorVisualData visual = meta.getActorVisualData(actor->getId(), contains);
    if (!contains) {
        return procItem;
    }
    QPointF p = visual.getPos(contains);
    if (contains) {
        procItem->setPos(p);
    }
    QString s = visual.getStyle(contains);
    if (contains) {
        procItem->setStyle(s);
        {
            ItemViewStyle *eStyle = procItem->getStyleById(ItemStyles::EXTENDED);
            ItemViewStyle *sStyle = procItem->getStyleById(ItemStyles::SIMPLE);
            QColor c = visual.getColor(contains);
            if (contains) {
                eStyle->setBgColor(c);
                sStyle->setBgColor(c);
            }
            QFont f = visual.getFont(contains);
            if (contains) {
                eStyle->setDefaultFont(f);
                sStyle->setDefaultFont(f);
            }
            QRectF r = visual.getRect(contains);
            if (contains) {
                qobject_cast<ExtendedProcStyle*>(eStyle)->setFixedBounds(r);
            }
        }
    }
    foreach (WorkflowPortItem *portItem, procItem->getPortItems()) {
        Port *port = portItem->getPort();
        qreal a = visual.getPortAngle(port->getId(), contains);
        if (contains) {
            portItem->setOrientation(a);
        }
    }
    return procItem;
}

void SceneCreator::createBus(const QMap<Port*, WorkflowPortItem*> &ports, Link *link) {
    WorkflowPortItem *src = ports[link->source()];
    WorkflowPortItem *dst = ports[link->destination()];
    WorkflowBusItem *busItem = scene->addFlow(src, dst, link);
    ActorId srcActorId = src->getOwner()->getProcess()->getId();
    ActorId dstActorId = dst->getOwner()->getProcess()->getId();

    bool contains = false;
    QPointF p = meta.getTextPos(srcActorId, link->source()->getId(),
        dstActorId, link->destination()->getId(), contains);
    if (contains) {
        busItem->getText()->setPos(p);
    }
}

}//namespace
