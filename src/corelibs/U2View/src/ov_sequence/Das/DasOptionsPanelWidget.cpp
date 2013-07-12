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

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>

#include <U2Gui/RegionSelector.h>
#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Gui/CreateAnnotationWidgetController.h>

namespace U2 {

#define DAS_UNIPROT "dasuniprot"    // hardcoded, taken from U2Core/DASSource.cpp
// TODO: add combobox with all available sources.

const QString DasOptionsPanelWidget::EXACT_SEARCH = tr("Exact sequence");
const QString DasOptionsPanelWidget::BLAST_SEARCH = tr("BLAST");
const QString DasOptionsPanelWidget::ALGORITHM_SETTINGS = tr("Algorithm settings");
const QString DasOptionsPanelWidget::ANNOTATIONS_SETTINGS = tr("Annotations settings");
const QString DasOptionsPanelWidget::SOURCES = tr("DAS features sources");

DasBlastSettingsWidget::DasBlastSettingsWidget(QWidget* parent) : QWidget(parent) {
    setupUi(this);
}

int DasBlastSettingsWidget::getIdentity() {
    return identitySpinBox->value();
}

DasOptionsPanelWidget::DasOptionsPanelWidget(AnnotatedDNAView* adv) :
    annotatedDnaView(adv),
    ctx(adv->getSequenceInFocus()),
    selection(NULL),
    regionSelector(NULL),
    blastSettingsWidget(NULL),
    dasFeaturesListWidget(NULL),
    annotationsWidgetController(NULL) {
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
    GetDasIdsBySequenceTask* searchIdsTask = NULL;
    if (searchTypeComboBox->currentText() == EXACT_SEARCH) {
        searchIdsTask = new GetDasIdsByExactSequenceTask(ctx->getSequenceData(U2_REGION_MAX));
    } else if (searchTypeComboBox->currentText() == BLAST_SEARCH) {
        searchIdsTask = new GetDasIdsByBlastTask(ctx->getSequenceData(U2_REGION_MAX), blastSettingsWidget->getIdentity());
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
    GetDasIdsBySequenceTask* searchIdsTask = qobject_cast<GetDasIdsBySequenceTask*>(sender());
    SAFE_POINT(searchIdsTask, "Sender is not defined", );

    if (searchIdsTask->isFinished()) {
        idList->clear();
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
    delete regionSelectorContainerLayout->takeAt(0);

    qint64 seqLength = ctx->getSequenceLength();
    selection = ctx->getSequenceSelection();
    regionSelector = new RegionSelector(regionSelectorContainerWidget, seqLength, true, selection);

    // Update annotations settings widget
    CreateAnnotationModel cm;
    cm.hideLocation = true;
    cm.sequenceObjectRef = ctx->getSequenceObject();
    cm.sequenceLen = ctx->getSequenceLength();
    annotationsWidgetController->updateWidgetForAnnotationModel(cm);

    checkState();
}

void DasOptionsPanelWidget::initialize() {
    SAFE_POINT(NULL != ctx, "Active sequence context is NULL.", );

    qint64 seqLength = ctx->getSequenceLength();
    selection = ctx->getSequenceSelection();
    regionSelector = new RegionSelector(regionSelectorContainerWidget,
                                        seqLength,
                                        true,
                                        selection);

    regionSelectorContainerLayout->addWidget(regionSelector);

    blastSettingsWidget = new DasBlastSettingsWidget();
    settingsContainerLayout->addWidget(new ShowHideSubgroupWidget(ALGORITHM_SETTINGS, ALGORITHM_SETTINGS, blastSettingsWidget, false));

    searchTypeComboBox->addItem(EXACT_SEARCH);
    searchTypeComboBox->addItem(BLAST_SEARCH);
    searchTypeComboBox->setCurrentIndex(0);

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
}

void DasOptionsPanelWidget::checkState() {
    SAFE_POINT(ctx, "Active sequence context is NULL.", );
    blastSettingsWidget->setEnabled(searchTypeComboBox->currentText() == BLAST_SEARCH);

    bool ok = false;
    regionSelector->getRegion(&ok);
    searchIdsButton->setEnabled(ok);

    bool annotationButtonIsEnabled = true;
    annotationButtonIsEnabled &= annotationsWidgetController->validate().isEmpty();
    annotationButtonIsEnabled &= !idList->selectedItems().isEmpty();
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

}   // namespace
