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

#include "DasOptionsPanelWidget.h"

#include <U2Core/U2SafePoints.h>
#include <U2Core/PicrApiTask.h>
#include <U2Core/UniprotBlastTask.h>
#include <U2Core/LoadDASDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/MultiTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/Settings.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/GUrlUtils.h>

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>

#include <U2Gui/OpenViewTask.h>
#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Gui/GUIUtils.h>

namespace U2 {

#define SAVE_DIR QString("downloadremotefiledialog/savedir")    // taken from U2Core/DownloadRemoteFileDialog.cpp
#define DAS_UNIPROT "dasuniprot"    // hardcoded, taken from U2Core/DASSource.cpp
// TODO: add combobox with all available sources.

const QString DasOptionsPanelWidget::EXACT_SEARCH = tr("Exact sequence");
const QString DasOptionsPanelWidget::BLAST_SEARCH = tr("BLAST");
const QString DasOptionsPanelWidget::ALGORITHM_SETTINGS = tr("Algorithm settings");
const QString DasOptionsPanelWidget::ANNOTATIONS_SETTINGS = tr("Annotations settings");
const QString DasOptionsPanelWidget::SOURCES = tr("DAS features sources");
const QString DasOptionsPanelWidget::WHOLE_SEQUENCE = tr("Whole sequence");
const QString DasOptionsPanelWidget::SELECTED_REGION = tr("Selected region");
const QString DasOptionsPanelWidget::CUSTOM_REGION = tr("Custom region");

DasBlastSettingsWidget::DasBlastSettingsWidget(QWidget* parent) : QWidget(parent) {
    setupUi(this);
    minimumIdentityDoubleSpinBox->setValue(90);

    databaseComboBox->addItem("UniProtKB", "uniprotkb");
    databaseComboBox->addItem("...Archaea", "uniprotkb_archaea");
    databaseComboBox->addItem("...Bacteria", "uniprotkb_bacteria");
    databaseComboBox->addItem("...Eucaryota", "uniprotkb_eukaryota");
    databaseComboBox->addItem("...Arthropoda", "uniprotkb_arthropoda");
    databaseComboBox->addItem("...Fungi", "uniprotkb_fungi");
    databaseComboBox->addItem("...Human", "uniprotkb_human");
    databaseComboBox->addItem("...Mammals", "uniprotkb_mammals");
    databaseComboBox->addItem("...Nematoda", "uniprotkb_nematoda");
    databaseComboBox->addItem("...Plants", "uniprotkb_plants");
    databaseComboBox->addItem("...Rodents", "uniprotkb_rodents");
    databaseComboBox->addItem("...Vertebrates", "uniprotkb_vertebrates");
    databaseComboBox->addItem("...Viruses", "uniprotkb_viruses");
    databaseComboBox->addItem("...PDB", "uniprotkb_pdb");
    databaseComboBox->addItem("...Complete microbial proteoms", "uniprotkb_complete_microbial_proteomes");
    databaseComboBox->addItem("UniProtKB/Swiss-Prot", "uniprotkb_swissprot");
    databaseComboBox->addItem("UniRef100", "UniRef100");
    databaseComboBox->addItem("UniRef90", "UniRef90");
    databaseComboBox->addItem("UniRef50", "UniRef50");
    databaseComboBox->addItem("UniParc", "uniparc");

    matrixComboBox->addItem("Auto", "");
    matrixComboBox->addItem("BLOSUM-45", "blosum45");
    matrixComboBox->addItem("BLOSUM-62", "blosum62");
    matrixComboBox->addItem("BLOSUM-80", "blosum80");
    matrixComboBox->addItem("PAM-70", "pam70");
    matrixComboBox->addItem("PAM-30", "pam30");

    filteringComboBox->addItem("None", "false");
    filteringComboBox->addItem("Filter low complexity regions", "true");
    filteringComboBox->addItem("Mask lookup table only", "mask");

    thresholdComboBox->addItems(UniprotBlastSettings::ALLOWED_THRESHOLD);
    gappedComboBox->addItems(UniprotBlastSettings::ALLOWED_GAPPED);
    hitsComboBox->addItems(UniprotBlastSettings::ALLOWED_HITS);

    databaseComboBox->setCurrentIndex(databaseComboBox->findData(UniprotBlastSettings::DEFAULT_DATABASE));
    thresholdComboBox->setCurrentIndex(thresholdComboBox->findText(UniprotBlastSettings::DEFAULT_THRESHOLD));
    matrixComboBox->setCurrentIndex(matrixComboBox->findData(UniprotBlastSettings::DEFAULT_MATRIX));
    filteringComboBox->setCurrentIndex(filteringComboBox->findData(UniprotBlastSettings::DEFAULT_FILTERING));
    gappedComboBox->setCurrentIndex(gappedComboBox->findText(UniprotBlastSettings::DEFAULT_GAPPED));
    hitsComboBox->setCurrentIndex(hitsComboBox->findText(UniprotBlastSettings::DEFAULT_HITS));
}

UniprotBlastSettings DasBlastSettingsWidget::getSettings() {
    UniprotBlastSettings settings;
    settings.insert(UniprotBlastSettings::DATABASE, databaseComboBox->itemData(databaseComboBox->currentIndex()).toString());
    settings.insert(UniprotBlastSettings::THRESHOLD, thresholdComboBox->currentText());
    settings.insert(UniprotBlastSettings::MATRIX, matrixComboBox->itemData(matrixComboBox->currentIndex()).toString());
    settings.insert(UniprotBlastSettings::FILTERING, filteringComboBox->itemData(filteringComboBox->currentIndex()).toString());
    settings.insert(UniprotBlastSettings::GAPPED, gappedComboBox->currentText());
    settings.insert(UniprotBlastSettings::HITS, hitsComboBox->currentText());
    return settings;
}

DasOptionsPanelWidget::DasOptionsPanelWidget(AnnotatedDNAView* adv) :
    annotatedDnaView(adv),
    ctx(adv->getSequenceInFocus()),
    selection(NULL),
    settingsShowHideWidget(NULL),
    blastSettingsWidget(NULL),
    dasFeaturesListWidget(NULL),
    annotationsWidgetController(NULL),
    regionSelector(NULL),
    fetchIdsAction(NULL),
    fetchAnnotationsAction(NULL),
    openInNewViewAction(NULL) {
    setupUi(this);
    initialize();
    connectSignals();
    checkState();
}

void DasOptionsPanelWidget::sl_searchTypeChanged(int type) {
    Q_UNUSED(type);
    checkState();
}

void DasOptionsPanelWidget::sl_searchIdsClicked() {
    if (NULL == ctx) {
        return;
    }

    idList->clearContents();
    GetDasIdsBySequenceTask* searchIdsTask = NULL;

    if (searchTypeComboBox->currentText() == EXACT_SEARCH) {
        searchIdsTask = new GetDasIdsByExactSequenceTask(ctx->getSequenceData(getRegion()));
        connect(searchIdsTask,
                SIGNAL(si_stateChanged()),
                SLOT(sl_exactSearchFinish()));
        AppContext::getTaskScheduler()->registerTopLevelTask(searchIdsTask);
    } else if (searchTypeComboBox->currentText() == BLAST_SEARCH) {
        SAFE_POINT (NULL != blastSettingsWidget, "BLAST settings widget is null", );
        UniprotBlastTask* blastTask = new UniprotBlastTask(ctx->getSequenceData(getRegion()), blastSettingsWidget->getSettings());
        connect(blastTask,
                SIGNAL(si_stateChanged()),
                SLOT(sl_blastSearchFinish()));
        AppContext::getTaskScheduler()->registerTopLevelTask(blastTask);
    } else {
        FAIL("Unexpected search type", );
    }
}

void DasOptionsPanelWidget::sl_annotateClicked() {
    annotationData.clear();
    QList<DASSource> featureSources = getFeatureSources();

    QString accessionNumber = idList->item(idList->currentRow(), 0)->text();

    loadDasObjectTasks.clear();
    foreach (DASSource featureSource, featureSources) {
        LoadDASObjectTask * loadAnnotationsTask = new LoadDASObjectTask(accessionNumber, featureSource, DASFeatures);
        connect(loadAnnotationsTask,
                SIGNAL(si_stateChanged()),
                SLOT(sl_onLoadAnnotationsFinish()));
        loadDasObjectTasks << loadAnnotationsTask;
    }

    AppContext::getTaskScheduler()->registerTopLevelTask(new MultiTask("Load DAS annotations for current sequence", loadDasObjectTasks));
}

void DasOptionsPanelWidget::sl_exactSearchFinish() {
    GetDasIdsBySequenceTask* searchIdsTask = qobject_cast<GetDasIdsBySequenceTask*>(sender());
    SAFE_POINT(searchIdsTask, "Sender is not defined", );

    if (searchIdsTask->isFinished()) {
        idList->clearContents();
        QList<PicrElement> results = searchIdsTask->getResults();
        for (int i = 0; i < results.count(); ++i) {
            idList->insertRow(i);
            idList->setItem(i, 0, new QTableWidgetItem(results[i].accessionNumber));
            idList->setItem(i, 1, new QTableWidgetItem("100%"));
        }

        idList->setCurrentCell(0, 0);
        checkState();
    }
}

void DasOptionsPanelWidget::sl_blastSearchFinish() {
    UniprotBlastTask* blastTask = qobject_cast<UniprotBlastTask*>(sender());
    SAFE_POINT(blastTask, "Sender is not defined", );

    if (blastTask->isFinished()) {
        idList->clearContents();
        QList<UniprotResult> results = blastTask->getResults();
        for (int i = 0; i < results.count(); ++i) {
            if (results[i].identity >= blastSettingsWidget->getMinIdentity()) {
                idList->insertRow(i);
                idList->setItem(i, 0, new QTableWidgetItem(results[i].accession));
                idList->setItem(i, 1, new QTableWidgetItem(QString::number(results[i].identity) + "%"));
            }
        }

        idList->setCurrentCell(0, 0);
        checkState();
    }
}

void DasOptionsPanelWidget::sl_onLoadAnnotationsFinish() {
    LoadDASObjectTask* loadDasObjectTask = qobject_cast<LoadDASObjectTask*>(sender());
    SAFE_POINT(loadDasObjectTask, "Sender is not defined", );

    if (loadDasObjectTask->isFinished()) {
        loadDasObjectTasks.removeAll(loadDasObjectTask);
        mergeFeatures(loadDasObjectTask->getAnnotationData());
    }

    if (loadDasObjectTasks.isEmpty()) {
        addAnnotations();
    }
}

void DasOptionsPanelWidget::sl_onSequenceFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*) {
    ctx = annotatedDnaView->getSequenceInFocus();
    if (NULL == ctx) {
        return;
    }

    // Update region selector widget
    disconnect(SIGNAL(si_onSelectionChanged(GSelection*)),
               this,
               SLOT(sl_onSelectionChanged(GSelection*)));

    selection = ctx->getSequenceSelection();

    connect(selection,
            SIGNAL(si_selectionChanged(LRegionsSelection*, QVector<U2Region>, QVector<U2Region>)),
            SLOT(sl_onSelectionChanged(LRegionsSelection*, QVector<U2Region>, QVector<U2Region>)));

    // Update annotations settings widget
    CreateAnnotationModel cm;
    cm.hideLocation = true;
    cm.sequenceObjectRef = ctx->getSequenceObject();
    cm.sequenceLen = ctx->getSequenceLength();
    cm.hideAnnotationParameters = true;
    annotationsWidgetController->updateWidgetForAnnotationModel(cm);

    checkState();
}

void DasOptionsPanelWidget::sl_onSelectionChanged(LRegionsSelection* _selection, const QVector<U2Region>& added, const QVector<U2Region>& removed) {
    Q_UNUSED(added);
    Q_UNUSED(removed);
    SAFE_POINT(selection == _selection, "Selection is invalid", );
    updateRegionSelectorWidget();
    checkState();
}

void DasOptionsPanelWidget::sl_openInNewView() {
    QString dir = AppContext::getSettings()->getValue(SAVE_DIR, "").value<QString>();
    if (dir.isEmpty()) {
        dir = LoadRemoteDocumentTask::getDefaultDownloadDirectory();
    }
    SAFE_POINT(!dir.isEmpty(), "Default download dir is empty", );

    U2OpStatus2Log os;
    dir = GUrlUtils::prepareDirLocation(dir, os);
    SAFE_POINT(!dir.isEmpty(), "Prepared default download dir is empty", );

    QString accessionNumber = idList->currentItem()->text();

    DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
    SAFE_POINT(NULL != dasRegistry, "DAS registry is NULL", );

    QList<DASSource> featureSources = getFeatureSources();
    DASSource refSource = getSequenceSource();
    SAFE_POINT(refSource.isValid(), "Reference source is invalid", );

    AppContext::getTaskScheduler()->registerTopLevelTask(new LoadDASDocumentsAndOpenViewTask(accessionNumber, dir, refSource, featureSources, false));
}

void DasOptionsPanelWidget::initialize() {
    SAFE_POINT(NULL != ctx, "Active sequence context is NULL.", );

    selection = ctx->getSequenceSelection();

    regionSelector = new RegionSelector(this, ctx->getSequenceLength(), true, ctx->getSequenceSelection());
    regionLayout->addWidget(regionSelector);

    updateRegionSelectorWidget();

    blastSettingsWidget = new DasBlastSettingsWidget();
    settingsShowHideWidget = new ShowHideSubgroupWidget(ALGORITHM_SETTINGS, ALGORITHM_SETTINGS, blastSettingsWidget, false);
    settingsContainerLayout->addWidget(settingsShowHideWidget);

    searchTypeComboBox->addItem(EXACT_SEARCH);
    searchTypeComboBox->addItem(BLAST_SEARCH);
    searchTypeComboBox->setCurrentIndex(searchTypeComboBox->findText(BLAST_SEARCH));

    // DAS sources
    dasFeaturesListWidget = new QListWidget();
    dasFeaturesListWidget->setMaximumHeight(100);
    dasFeaturesListWidget->setMinimumHeight(100);
    sourcesContainerLayout->addWidget(new ShowHideSubgroupWidget(SOURCES, SOURCES, dasFeaturesListWidget, false));

    DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
    SAFE_POINT(dasRegistry, "DAS registry is NULL", );
    const DASSource& dasSource = dasRegistry->findById(DAS_UNIPROT);
    if (dasSource.isValid()){
        dasFeaturesListWidget->clear();
        const QList<DASSource>& featureSources = dasRegistry->getFeatureSourcesByType(dasSource.getReferenceType());
        foreach(const DASSource& s, featureSources){
            QListWidgetItem* item = new QListWidgetItem(s.getName());
            item->setData(Qt::UserRole, s.getId());
            item->setToolTip(s.getHint());
            item->setCheckState(Qt::Checked);
            dasFeaturesListWidget->addItem(item);
        }
    }

    // Annotations widget
    CreateAnnotationModel cm;
    cm.hideLocation = true;
    cm.sequenceObjectRef = ctx->getSequenceObject();
    cm.sequenceLen = ctx->getSequenceLength();
    cm.hideAnnotationParameters = true;
    annotationsWidgetController = new CreateAnnotationWidgetController(cm, this, optPanel);

    annotationsSettingsContainerLayout->addWidget(new ShowHideSubgroupWidget(ANNOTATIONS_SETTINGS, ANNOTATIONS_SETTINGS, annotationsWidgetController->getWidget(), false));

    // Setup context menu of the idList widget
    idList->setContextMenuPolicy(Qt::ActionsContextMenu);
    fetchIdsAction = new QAction("Fetch IDs", idList);
    fetchAnnotationsAction = new QAction("Fetch Annotations", idList);
    openInNewViewAction = new QAction("Open in a new view", idList);
    idList->addAction(fetchIdsAction);
    idList->addAction(fetchAnnotationsAction);
    idList->addAction(openInNewViewAction);

    idList->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    idList->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
}

void DasOptionsPanelWidget::connectSignals() {
    connect(searchTypeComboBox,
            SIGNAL(currentIndexChanged(int)),
            SLOT(sl_searchTypeChanged(int)));
    connect(searchIdsButton,
            SIGNAL(clicked()),
            SLOT(sl_searchIdsClicked()));
    connect(annotateButton,
            SIGNAL(clicked()),
            SLOT(sl_annotateClicked()));
    connect(annotatedDnaView,
            SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)),
            SLOT(sl_onSequenceFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*)));
    connect(selection,
            SIGNAL(si_selectionChanged(LRegionsSelection*, QVector<U2Region>, QVector<U2Region>)),
            SLOT(sl_onSelectionChanged(LRegionsSelection*, QVector<U2Region>, QVector<U2Region>)));
    connect(fetchIdsAction,
            SIGNAL(triggered()),
            SLOT(sl_searchIdsClicked()));
    connect(fetchAnnotationsAction,
            SIGNAL(triggered()),
            SLOT(sl_annotateClicked()));
    connect(openInNewViewAction,
            SIGNAL(triggered()),
            SLOT(sl_openInNewView()));
}

void DasOptionsPanelWidget::checkState() {
    SAFE_POINT(ctx, "Active sequence context is NULL.", );
    settingsShowHideWidget->setVisible(searchTypeComboBox->currentText() == BLAST_SEARCH);

    bool ok = regionIsOk();
    DNAAlphabet* alphabet = ctx->getAlphabet();
    SAFE_POINT(alphabet != NULL, "DasOptionsPanelWidget::checkState", )
    ok &= alphabet->isAmino();

    searchIdsButton->setEnabled(ok);
    fetchIdsAction->setEnabled(ok);

    bool annotationButtonIsEnabled = true;
    annotationButtonIsEnabled &= annotationsWidgetController->validate().isEmpty();
    annotationButtonIsEnabled &= !idList->selectedItems().isEmpty();
    annotationButtonIsEnabled &= ctx->getAlphabet()->isAmino();
    annotateButton->setEnabled(annotationButtonIsEnabled);
    fetchAnnotationsAction->setEnabled(annotationButtonIsEnabled);

    bool openActionIsEnabled = true;
    openActionIsEnabled &= !idList->selectedItems().isEmpty();
    openActionIsEnabled &= ctx->getAlphabet()->isAmino();
    openInNewViewAction->setEnabled(openActionIsEnabled);
}

QList<DASSource> DasOptionsPanelWidget::getFeatureSources() {
    QList<DASSource> featureSources;

    DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
    SAFE_POINT(dasRegistry, "DAS registry pointer is NULL", featureSources);

    for (int i = 0; i < dasFeaturesListWidget->count(); i++) {
        QListWidgetItem* item = dasFeaturesListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            QString featureId = item->data(Qt::UserRole).toString();
            DASSource fSource = dasRegistry->findById(featureId);
            if (fSource.isValid()) {
                featureSources.append(fSource);
            }
        }
    }

    return featureSources;
}

DASSource DasOptionsPanelWidget::getSequenceSource() {
    DASSource refSource;

    DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
    SAFE_POINT(dasRegistry, "DAS registry pointer is NULL", refSource);

    refSource = dasRegistry->findById(DAS_UNIPROT);
    SAFE_POINT(refSource.isValid(), "Reference source is innvalid: %1", DASSource());

    return refSource;
}

void DasOptionsPanelWidget::mergeFeatures(const QMap<QString, QList<SharedAnnotationData> >& newAnnotations) {
    const QStringList& keys =  newAnnotations.keys();
    foreach (const QString& key, keys) {
        if (annotationData.contains(key)) {
            const QList<SharedAnnotationData>& curList = annotationData[key];
            const QList<SharedAnnotationData>& tomergeList = newAnnotations[key];
            foreach (SharedAnnotationData d, tomergeList) {
                if (!curList.contains(d)) {
                    annotationData[key].append(d);
                }
            }
        } else {
            annotationData.insert(key, newAnnotations[key]);
        }
    }
}

void DasOptionsPanelWidget::addAnnotations() {
    if (annotationData.isEmpty()) {
        return;
    }

    SAFE_POINT(ctx, "Current sequence context is NULL", );

    bool annObjectIsOk = annotationsWidgetController->prepareAnnotationObject();
    SAFE_POINT(annObjectIsOk, "Cannot create an annotation object. Please check settings", );

    const CreateAnnotationModel& cm = annotationsWidgetController->getModel();
    AnnotationTableObject* annotationTableObject = cm.getAnnotationObject();

    foreach (const QString& grname, annotationData.keys()) {
        const QList<SharedAnnotationData> sdata = annotationData[grname];
        if (!sdata.isEmpty()) {
            foreach (SharedAnnotationData d, sdata) {
                Annotation* a = new Annotation(d);
                //setRegion
                const U2Location& location = a->getLocation();
                if (location->isSingleRegion() && location->regions.first() == U2_REGION_MAX) {
                    U2Location newLoc = location;
                    newLoc->regions.clear();
                    newLoc->regions.append(U2Region(0, ctx->getSequenceLength()));
                    a->setLocation(newLoc);
                }
                annotationTableObject->getRootGroup()->getSubgroup(grname, true)->addAnnotation(a);
            }
        }
    }

    annotationsWidgetController->updateWidgetForAnnotationModel(cm);
}


U2Region DasOptionsPanelWidget::getRegion() {
    if (regionIsOk()) {
        U2Region region = regionSelector->getRegion();
        return region;
    }

    return U2Region(0, ctx->getSequenceLength());
}

bool DasOptionsPanelWidget::regionIsOk() {
    bool result=false;
    U2Region region = regionSelector->getRegion(&result);
    return result;
}

void DasOptionsPanelWidget::updateRegionSelectorWidget(){
    SAFE_POINT(NULL != selection, "Selection is NULL", );
}


}   // namespace
