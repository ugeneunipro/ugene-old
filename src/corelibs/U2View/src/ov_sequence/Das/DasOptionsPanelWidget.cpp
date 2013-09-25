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
#include <U2Core/L10n.h>

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

const QString DasOptionsPanelWidget::BLAST_SEARCH = tr("BLAST");
const QString DasOptionsPanelWidget::ALGORITHM_SETTINGS = tr("Algorithm settings");
const QString DasOptionsPanelWidget::ANNOTATIONS_SETTINGS = tr("Annotations settings");
const QString DasOptionsPanelWidget::SOURCES = tr("DAS features sources");
const QString DasOptionsPanelWidget::WHOLE_SEQUENCE = tr("Whole sequence");
const QString DasOptionsPanelWidget::SELECTED_REGION = tr("Selected region");
const QString DasOptionsPanelWidget::CUSTOM_REGION = tr("Custom region");

const static QString SHOW_OPTIONS_LINK("show_options_link");

#define MIN_SEQ_LENGTH 4
#define MAX_SEQ_LENGTH 1900 //because a GET request can handle only 2048 characters

DasBlastSettingsWidget::DasBlastSettingsWidget(QWidget* parent) : QWidget(parent) {
    setupUi(this);

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

    thresholdComboBox->setCurrentIndex(thresholdComboBox->findText(UniprotBlastSettings::DEFAULT_THRESHOLD));
    matrixComboBox->setCurrentIndex(matrixComboBox->findData(UniprotBlastSettings::DEFAULT_MATRIX));
    filteringComboBox->setCurrentIndex(filteringComboBox->findData(UniprotBlastSettings::DEFAULT_FILTERING));
    gappedComboBox->setCurrentIndex(gappedComboBox->findText(UniprotBlastSettings::DEFAULT_GAPPED));
    hitsComboBox->setCurrentIndex(hitsComboBox->findText(UniprotBlastSettings::DEFAULT_HITS));
}

UniprotBlastSettings DasBlastSettingsWidget::getSettings() {
    UniprotBlastSettings settings;
    settings.insert(UniprotBlastSettings::THRESHOLD, thresholdComboBox->currentText());
    settings.insert(UniprotBlastSettings::MATRIX, matrixComboBox->itemData(matrixComboBox->currentIndex()).toString());
    settings.insert(UniprotBlastSettings::FILTERING, filteringComboBox->itemData(filteringComboBox->currentIndex()).toString());
    settings.insert(UniprotBlastSettings::GAPPED, gappedComboBox->currentText());
    settings.insert(UniprotBlastSettings::HITS, hitsComboBox->currentText());
    return settings;
}

DasOptionsPanelWidget::DasOptionsPanelWidget(AnnotatedDNAView* adv)
:annotatedDnaView(adv)
,ctx(adv->getSequenceInFocus())
,selection(NULL)
,settingsShowHideWidget(NULL)
,blastSettingsWidget(NULL)
,dasFeaturesListWidget(NULL)
,annotationsWidgetController(NULL)
,regionSelector(NULL)
,fetchIdsAction(NULL)
,fetchAnnotationsAction(NULL)
,openInNewViewAction(NULL)
,showMore(true)
,getIdsTask(NULL)
{
    setupUi(this);
    initialize();
    connectSignals();
    checkState();
    updateShowOptions();
}

void DasOptionsPanelWidget::sl_searchTypeChanged(int type) {
    Q_UNUSED(type);
    checkState();
}

void DasOptionsPanelWidget::sl_searchIdsClicked() {
    if (NULL == ctx) {
        return;
    }

    SAFE_POINT (NULL != blastSettingsWidget, "BLAST settings widget is null", );

    if (getIdsTask == NULL || getIdsTask->isCanceled() || getIdsTask->isFinished()){
        getIdsTask = new UniprotBlastTask(ctx->getSequenceData(getRegion()), blastSettingsWidget->getSettings());
        connect(getIdsTask,
            SIGNAL(si_stateChanged()),
            SLOT(sl_blastSearchFinish()));
        AppContext::getTaskScheduler()->registerTopLevelTask(getIdsTask);
    }
    checkState();
}

void DasOptionsPanelWidget::sl_loadAnnotations() {
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

void DasOptionsPanelWidget::sl_blastSearchFinish() {
    UniprotBlastTask* blastTask = qobject_cast<UniprotBlastTask*>(sender());
    SAFE_POINT(blastTask, "Sender is not defined", );

    if (blastTask != getIdsTask){
        //context was switched while fetching IDs. I caused deleting of the widget
        getIdsTask = blastTask;
    }

    checkState();
    
    if (getIdsTask == NULL || getIdsTask->isCanceled() || getIdsTask->hasError()){
        getIdsTask = NULL;
        return;
    }

    if (getIdsTask->isFinished()) {
        UniprotBlastTask* blastTask = qobject_cast<UniprotBlastTask*>(getIdsTask);
        if (blastTask == NULL){
            getIdsTask = NULL;
            return;
        }
        
        clearTableContent();
        
        QList<UniprotResult> results = blastTask->getResults();
        for (int i = 0; i < results.count(); ++i) {
            if (results[i].identity >= getMinIdentity()) {
                int rowNumber = idList->rowCount();
                idList->insertRow(rowNumber);
                idList->setItem(rowNumber, 0, new QTableWidgetItem(results[i].accession));
                idList->setItem(rowNumber, 1, new QTableWidgetItem(QString::number(results[i].identity) + "%"));
            }
        }

        idList->setCurrentCell(0, 0);

        getIdsTask = NULL;
        checkState();
    }
}

void DasOptionsPanelWidget::sl_onLoadAnnotationsFinish() {
    LoadDASObjectTask* loadDasObjectTask = qobject_cast<LoadDASObjectTask*>(sender());
    SAFE_POINT(loadDasObjectTask, "Sender is not defined", );

    if (loadDasObjectTask->isFinished() && loadDasObjectTasks.contains(loadDasObjectTask)) {
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

    //clear no sense results and tasks if the sequence is switched
    clear();

    checkState();
}

void DasOptionsPanelWidget::sl_onSelectionChanged(LRegionsSelection* _selection, const QVector<U2Region>& added, const QVector<U2Region>& removed) {
    Q_UNUSED(added);
    Q_UNUSED(removed);
    SAFE_POINT(selection == _selection, "Selection is invalid", );
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

void DasOptionsPanelWidget::sl_showLessClicked(const QString& link) {
    SAFE_POINT(SHOW_OPTIONS_LINK == link, "Incorrect link!",);
    updateShowOptions();
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

    databaseComboBox->setCurrentIndex(databaseComboBox->findData(UniprotBlastSettings::DEFAULT_DATABASE));

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

    //disable editing of the results
    idList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //only one ID can be selected for now
    idList->setSelectionMode(QAbstractItemView::SingleSelection);

    hintLabel->hide();
    hintLabel->setStyleSheet(
        "color: " + L10N::errorColorLabelStr() + ";"
        "font: bold;");
}

void DasOptionsPanelWidget::connectSignals() {
    connect(searchIdsButton,
            SIGNAL(clicked()),
            SLOT(sl_searchIdsClicked()));
    connect(annotateButton,
            SIGNAL(clicked()),
            SLOT(sl_loadAnnotations()));
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
            SLOT(sl_loadAnnotations()));
    connect(openInNewViewAction,
            SIGNAL(triggered()),
            SLOT(sl_openInNewView()));
    connect(lblShowMoreLess,
            SIGNAL(linkActivated(const QString&)),
            SLOT(sl_showLessClicked(const QString&)));
     connect(regionSelector ,
            SIGNAL(si_regionChanged(const U2Region&)),
            SLOT(sl_onRegionChanged(const U2Region& )));
     connect(idList, 
            SIGNAL( doubleClicked ( const QModelIndex &  ) ),
            SLOT( sl_idDoubleClicked (const QModelIndex & ) ));
}

void DasOptionsPanelWidget::checkState() {
    SAFE_POINT(ctx, "Active sequence context is NULL.", );

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

    if (ctx->getAlphabet()->getType() == DNAAlphabet_AMINO) {
        setEnabled(true);
        lblInfoMessage->hide();
    } else {
        setEnabled(false);
        lblInfoMessage->show();
    }

    if (getIdsTask == NULL || getIdsTask->isFinished() || getIdsTask->isCanceled()){
        searchIdsButton->setEnabled(true);
        fetchIdsAction->setEnabled(true);
    }else{
        searchIdsButton->setEnabled(false);
        fetchIdsAction->setEnabled(false);
    }
    
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
    qint64 seqLength = ctx->getSequenceLength();

    bool annObjectIsOk = annotationsWidgetController->prepareAnnotationObject();
    SAFE_POINT(annObjectIsOk, "Cannot create an annotation object. Please check settings", );

    const CreateAnnotationModel& cm = annotationsWidgetController->getModel();
    AnnotationTableObject* annotationTableObject = cm.getAnnotationObject();

    foreach (const QString& grname, annotationData.keys()) {
        const QList<SharedAnnotationData> sdata = annotationData[grname];
        if (!sdata.isEmpty()) {
            foreach (SharedAnnotationData d, sdata) {
                Annotation* a = new Annotation(d);
                
                const U2Location& location = a->getLocation();
                if (location->isSingleRegion() && location->regions.first() == U2_REGION_MAX) {
                    //setRegion for full region sequence
                    U2Location newLoc = location;
                    newLoc->regions.clear();
                    newLoc->regions.append(U2Region(0, ctx->getSequenceLength()));
                    a->setLocation(newLoc);
                }else{
                    //cut annotations with the start position out of the current sequence
                    if (location->regions.size() > 0 && location->regions.first().startPos >= seqLength){
                        continue;
                    }
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

void DasOptionsPanelWidget::updateShowOptions() {
    // Change the label
    QString linkText = showMore ? tr("Show more options") : tr("Show less options");
#ifndef Q_OS_MAC
    linkText = QString("<a href=\"%1\" style=\"color: palette(shadow)\">").arg(SHOW_OPTIONS_LINK)
               + linkText
               + QString("</a>");
#else
    linkText = QString("<a href=\"%1\" style=\"color: gray\">").arg(SHOW_OPTIONS_LINK)
               + linkText
               + QString("</a>");
#endif

    lblShowMoreLess->setText(linkText);
    lblShowMoreLess->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);

    // Show/hide the additional options
    if (showMore) {
        settingsContainerWidget->hide();
        sourcesContainerWidget->hide();
        annotationsSettingsContainerWidget->hide();
    }
    else {
        settingsContainerWidget->show();
        sourcesContainerWidget->show();
        annotationsSettingsContainerWidget->show();
    }
    // Change the mode
    showMore = !showMore;
}

void DasOptionsPanelWidget::clearTableContent(){
    
    idList->clearContents();
    while(idList->rowCount() > 0){
        idList->removeRow(0);
    }
    
}

DasOptionsPanelWidget::~DasOptionsPanelWidget(){
    clear();
}

void DasOptionsPanelWidget::clear(){
    clearTableContent();

    foreach(Task* loadDASTask, loadDasObjectTasks){
        if (loadDASTask != NULL && !loadDASTask->isFinished()){
            loadDASTask->cancel();
        }
    }
    loadDasObjectTasks.clear();

    annotationData.clear();

    if (getIdsTask != NULL && !getIdsTask->isFinished()){
        getIdsTask->cancel();
        getIdsTask = NULL;
    }
}

void DasOptionsPanelWidget::sl_onRegionChanged( const U2Region& r){
    clear();

    if (r.length < MIN_SEQ_LENGTH){
        hintLabel->setText(tr("Warning: Selected region is too short. It should be from 4 to 1900 amino acids."));
        hintLabel->show();
    }else if(r.length > MAX_SEQ_LENGTH){
        hintLabel->setText(tr("Warning: Selected region is too long. It should be from 4 to 1900 amino acids."));
        hintLabel->show();
    }else{
        hintLabel->hide();
    }
}

void DasOptionsPanelWidget::sl_idDoubleClicked( const QModelIndex & ){
    sl_loadAnnotations();
}


}   // namespace
