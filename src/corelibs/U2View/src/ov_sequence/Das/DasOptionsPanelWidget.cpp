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
#include <U2Core/LoadDASDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/MultiTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/DNAAlphabet.h>

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>

#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Gui/GUIUtils.h>

namespace U2 {

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
    initialize();
    connectSignals();
    checkState();
}

PicrBlastSettings DasBlastSettingsWidget::getSettings() {
    PicrBlastSettings settings;
    settings.insert(PicrBlastSettings::PROGRAM, programComboBox->currentText());
    settings.insert(PicrBlastSettings::IDENTITY, QString::number(identitySpinBox->value()));
    settings.insert(PicrBlastSettings::MATRIX, matrixComboBox->currentText());
    settings.insert(PicrBlastSettings::FILTER, filterComboBox->currentText());

    QStringList gapCosts = gapCostsComboBox->currentText().split(" ");
    SAFE_POINT(2 == gapCosts.count(), "Invalid gap costs value", PicrBlastSettings());
    settings.insert(PicrBlastSettings::GAP_OPEN, gapCosts.first());
    settings.insert(PicrBlastSettings::GAP_EXT, gapCosts.last());

    settings.insert(PicrBlastSettings::DROP_OFF, dropOffComboBox->currentText());

    QString boolValue = gapAlignCheckBox->isChecked() ? "true" : "false";
    settings.insert(PicrBlastSettings::GAP_ALIGN, boolValue);

    return settings;
}

void DasBlastSettingsWidget::sl_onMatrixChanged(int index) {
    Q_UNUSED(index);
    checkState();
}

void DasBlastSettingsWidget::initialize() {
    QStringList pam30 = QStringList() << "9 1"
                                      << "5 2"
                                      << "6 2"
                                      << "7 2"
                                      << "8 1"
                                      << "10 1";
    QStringList pam70 = QStringList() << "10 1"
                                      << "6 2"
                                      << "7 2"
                                      << "8 2"
                                      << "9 2"
                                      << "11 1";
    QStringList blosum45 = QStringList() << "15 2"
                                         << "10 3"
                                         << "11 3"
                                         << "12 3"
                                         << "12 2"
                                         << "13 2"
                                         << "14 2"
                                         << "16 2"
                                         << "15 1"
                                         << "16 1"
                                         << "17 1"
                                         << "18 1"
                                         << "19 1";
    QStringList blosum62 = QStringList() << "11 1"
                                         << "7 2"
                                         << "8 2"
                                         << "9 2"
                                         << "10 1"
                                         << "12 1";
    QStringList blosum80 = QStringList() << "10 1"
                                         << "6 2"
                                         << "7 2"
                                         << "8 2"
                                         << "9 1"
                                         << "11 1";

    gapCostsByMatrix.insert("PAM30", pam30);
    gapCostsByMatrix.insert("PAM70", pam70);
    gapCostsByMatrix.insert("BLOSUM45", blosum45);
    gapCostsByMatrix.insert("BLOSUM62", blosum62);
    gapCostsByMatrix.insert("BLOSUM80", blosum80);
}

void DasBlastSettingsWidget::connectSignals() {
    connect(matrixComboBox,
            SIGNAL(currentIndexChanged(int)),
            SLOT(sl_onMatrixChanged(int)));
}

void DasBlastSettingsWidget::checkState() {
    gapCostsComboBox->clear();

    QStringList gapCosts = gapCostsByMatrix.value(matrixComboBox->currentText());
    SAFE_POINT(0 != gapCosts.count(), "Unrecognized matrix", );

    gapCostsComboBox->addItems(gapCosts);
}


DasOptionsPanelWidget::DasOptionsPanelWidget(AnnotatedDNAView* adv) :
    annotatedDnaView(adv),
    ctx(adv->getSequenceInFocus()),
    selection(NULL),
    settingsShowHideWidget(NULL),
    blastSettingsWidget(NULL),
    dasFeaturesListWidget(NULL),
    annotationsWidgetController(NULL),
    regionSelector(NULL){
    setupUi(this);
    initialize();
    connectSignals();
    checkState();
}

void DasOptionsPanelWidget::sl_onSearchTypeChanged(int type) {
    Q_UNUSED(type);
    checkState();
}

void DasOptionsPanelWidget::sl_onSearchIdsClicked() {
    idList->clear();
    GetDasIdsBySequenceTask* searchIdsTask = NULL;

    if (searchTypeComboBox->currentText() == EXACT_SEARCH) {
        searchIdsTask = new GetDasIdsByExactSequenceTask(ctx->getSequenceData(getRegion()));
    } else if (searchTypeComboBox->currentText() == BLAST_SEARCH) {
        SAFE_POINT (NULL != blastSettingsWidget, "BLAST settings widget is null", );
        searchIdsTask = new GetDasIdsByBlastTask(ctx->getSequenceData(getRegion()), blastSettingsWidget->getSettings());
    } else {
        FAIL("Unexpected search type", );
    }

    connect(searchIdsTask,
            SIGNAL(si_stateChanged()),
            SLOT(sl_onSearchIdsFinish()));
    AppContext::getTaskScheduler()->registerTopLevelTask(searchIdsTask);
}

void DasOptionsPanelWidget::sl_onAnnotateClicked() {
    annotationData.clear();
    QList<DASSource> featureSources = getFeatureSources();

    QString accessionNumber = idList->currentItem()->text();

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

void DasOptionsPanelWidget::sl_onSearchIdsFinish() {
    idList->clear();
    GetDasIdsBySequenceTask* searchIdsTask = qobject_cast<GetDasIdsBySequenceTask*>(sender());
    SAFE_POINT(searchIdsTask, "Sender is not defined", );

    if (searchIdsTask->isFinished()) {
        QList<PicrElement> results = searchIdsTask->getResults();
        for (int i = 0; i < results.count(); ++i) {
            idList->addItem(new QListWidgetItem(results[i].accessionNumber));
        }

        idList->setCurrentRow(0);
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
    SAFE_POINT(NULL != ctx, "Active sequence context is NULL.", );

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
}

void DasOptionsPanelWidget::connectSignals() {
    connect(searchTypeComboBox,
            SIGNAL(currentIndexChanged(int)),
            SLOT(sl_onSearchTypeChanged(int)));
    connect(searchIdsButton,
            SIGNAL(clicked()),
            SLOT(sl_onSearchIdsClicked()));
    connect(annotateButton,
            SIGNAL(clicked()),
            SLOT(sl_onAnnotateClicked()));
    connect(annotatedDnaView,
            SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)),
            SLOT(sl_onSequenceFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*)));
    connect(selection,
            SIGNAL(si_selectionChanged(LRegionsSelection*, QVector<U2Region>, QVector<U2Region>)),
            SLOT(sl_onSelectionChanged(LRegionsSelection*, QVector<U2Region>, QVector<U2Region>)));
    
}

void DasOptionsPanelWidget::checkState() {
    SAFE_POINT(ctx, "Active sequence context is NULL.", );
    settingsShowHideWidget->setVisible(searchTypeComboBox->currentText() == BLAST_SEARCH);

    bool ok = regionIsOk();
    DNAAlphabet* alphabet = ctx->getAlphabet();
    SAFE_POINT(alphabet != NULL, "DasOptionsPanelWidget::checkState", )
    ok &= alphabet->isAmino();

    searchIdsButton->setEnabled(ok);

    bool annotationButtonIsEnabled = true;
    annotationButtonIsEnabled &= annotationsWidgetController->validate().isEmpty();
    annotationButtonIsEnabled &= !idList->selectedItems().isEmpty();
    ok &= ctx->getAlphabet()->isAmino();
    annotateButton->setEnabled(annotationButtonIsEnabled);
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
