/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "AnnotHighlightWidget.h"

#include <U2Core/AnnotationSelection.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/U2SafePoints.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/PanView.h>
#include <limits>

namespace U2 {

const QString ShowAllAnnotTypesLabel::SHOW_ALL_ANNOT_TYPES =
    ShowAllAnnotTypesLabel::tr("Show all annotation types");

const QString ShowAllAnnotTypesLabel::SHOW_ANNOT_TYPES_FOR_SEQ =
    ShowAllAnnotTypesLabel::tr("Show types for the sequence only");


ShowAllAnnotTypesLabel::ShowAllAnnotTypesLabel()
{
    // By default, show only types for the sequence
    showAllIsSelected = false;
    setText(SHOW_ALL_ANNOT_TYPES);

    setStyleSheet(
        "text-decoration: underline;"
        "color: gray;"
        "margin-left: 2px;"
        "margin-top: 1px;");
}

void ShowAllAnnotTypesLabel::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    if (showAllIsSelected) {
        showAllIsSelected = false;
        setText(SHOW_ALL_ANNOT_TYPES);
    }
    else {
        showAllIsSelected = true;
        setText(SHOW_ANNOT_TYPES_FOR_SEQ);
    }

    emit si_showAllStateChanged();
}


AnnotHighlightWidget::AnnotHighlightWidget(AnnotatedDNAView* _annotatedDnaView)
    : annotatedDnaView(_annotatedDnaView)
{
    SAFE_POINT(0 != annotatedDnaView, "AnnotatedDNAView is NULL!",);
    initLayout();
    connectSlots();
    loadAnnotTypes();
}

void AnnotHighlightWidget::initLayout()
{
    setMinimumSize(QSize(170, 150));

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);

    noAnnotTypesLabel = new QLabel("");
    noAnnotTypesLabel->setWordWrap(true);
    setNoAnnotTypesLabelValue();
    noAnnotTypesLabel->setStyleSheet("margin-left: 2px;");

    annotTreeTitle = new QLabel(tr("Select an annotation type:"));

    // Tree
    QVBoxLayout* treeLayout = new QVBoxLayout();
    treeLayout->setContentsMargins(0, 0, 0, 10);
    treeLayout->setSpacing(0);

    annotTree = new AnnotHighlightTree();

    showAllLabel = new ShowAllAnnotTypesLabel();

    treeLayout->addWidget(annotTree);
    treeLayout->addWidget(showAllLabel);

    // Configure settings
    QVBoxLayout* settingsLayout = new QVBoxLayout();
    settingsLayout->setContentsMargins(0, 0, 0, 0);
    settingsLayout->setSpacing(0);

    settingsTitle = new QLabel(tr("Configure the annotation type:"));
    annotSettingsWidget = new AnnotHighlightSettingsWidget();

    settingsLayout->addWidget(settingsTitle);
    settingsLayout->addWidget(annotSettingsWidget);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->setSpacing(0);

    prevAnnotationButton = new QPushButton(QIcon(":core/images/backward.png"), "");
    prevAnnotationButton->setFixedSize(32,32);
    prevAnnotationButton->setToolTip(AnnotHighlightWidget::tr("Previous annotation"));
    prevAnnotationButton->setDisabled(true);
    buttonsLayout->addWidget(prevAnnotationButton);
    buttonsLayout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    nextAnnotationButton = new QPushButton(QIcon(":core/images/forward.png"), "");
    nextAnnotationButton->setFixedSize(32,32);
    nextAnnotationButton->setToolTip(AnnotHighlightWidget::tr("Next annotation"));
    buttonsLayout->addWidget(nextAnnotationButton);
    if (annotatedDnaView->getAnnotationObjects(true).isEmpty()) {
        nextAnnotationButton->setDisabled(true);
    } else {
        sl_onAnnotationSelectionChanged();
    }

    // Init main layout
    mainLayout->addWidget(noAnnotTypesLabel);
    mainLayout->addWidget(annotTreeTitle);
    mainLayout->addLayout(treeLayout);
    mainLayout->addLayout(settingsLayout);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
}

bool AnnotHighlightWidget::isValidIndex(const QList<Annotation> &annotForNextPrev,
    int position)
{
    return !annotForNextPrev.empty() && position >= 0 && position < annotForNextPrev.size();
}

int AnnotHighlightWidget::searchAnnotWithEqualsStartPos(const QList<AnnotationTableObject *> &items,
    QList<Annotation> &annotForNextPrev, const Annotation *prev, int currentPosition)
{
    int qurIdx = -1;
    int currentIdx = qurIdx;
    foreach ( AnnotationTableObject *aTable, items ) {
        SAFE_POINT( NULL != aTable, "Invalid annotation table!", -1 );
        foreach ( const Annotation &annotation, aTable->getAnnotations( ) ) {
            const U2Location locData = annotation.getLocation( );
            foreach ( const U2Region &curPos, locData->regions ) {
                if ( curPos.startPos == currentPosition ) {
                    annotForNextPrev.append( annotation );
                    qurIdx++;
                }
            }
            if ( NULL != prev && *prev == annotation ) {
                currentIdx = qurIdx;
            }
        }
    }
    return currentIdx;
}

bool AnnotHighlightWidget::isNext( bool isForward, qint64 startPos, qint64 endPos, qint64 minPos ) {
    if ( isForward ) {
        return startPos > endPos && startPos < minPos;
    }
    return startPos < endPos && startPos > minPos;
}

qint64 AnnotHighlightWidget::searchNextPosition( const QList<AnnotationTableObject *> &items,
    int endPos, bool isForward, qint64& currentPosition)
{
    int locIdx = 0;
    qint64 MAX = std::numeric_limits<qint64>::max();
    qint64 minPos = MAX;
    if (!isForward) {
        minPos = -1;
    }
    foreach ( const AnnotationTableObject *aTable, items) {
        SAFE_POINT(aTable != NULL, "Invalid annotation table!", -1);
        foreach ( const Annotation &annotation, aTable->getAnnotations( ) ) {
            int locateIdx = 0;
            const U2Location locData = annotation.getLocation();
            foreach ( const U2Region &curPos, locData->regions ) {
                if (isNext(isForward, curPos.startPos, endPos, minPos)) {
                    minPos = curPos.startPos;
                    locIdx = locateIdx;
                }
                locateIdx++;
            }
        }
    }
    currentPosition = minPos;
    if (currentPosition == -1 || currentPosition == MAX) {
        currentPosition = -1;
        return -1;
    }
    return locIdx;
}

bool AnnotHighlightWidget::isFirstAnnotationRegion(const Annotation *annotation, const U2Region &region, bool fromTheBeginning) {
    const QList<AnnotationTableObject*> annotObjects = annotatedDnaView->getAnnotationObjects();
    QList<Annotation> annots;
    qint64 next = searchAnnotWithEqualsStartPos(annotObjects, annots, annotation, region.startPos);
    if (next != (fromTheBeginning ? 0: annots.size() - 1 )) {
        return false;
    }
    searchNextPosition(annotObjects, region.startPos, !fromTheBeginning, next);
    if (next != -1) {
        return false;
    }
    return true;

}

const Annotation * AnnotHighlightWidget::findFirstSelectedAnnotationRegion(qint64 &start, bool fromTheBeginning) {
    AnnotationSelection* as = annotatedDnaView->getAnnotationsSelection();
    CHECK(NULL != as, NULL);

    const QList<AnnotationSelectionData> selectionData = as->getSelection();
    start = -1;
    int annotationIdx = -1;
    int i = 0;
    foreach (AnnotationSelectionData selectionItem, selectionData) {
        foreach (U2Region region, selectionItem.getSelectedRegions()) {
            if (start == -1) {
                start = region.startPos;
                annotationIdx = i;
            } else {
                if ( (region.startPos - start) * (fromTheBeginning ? 1 : -1) > 0 ) {
                    start = region.startPos;
                    annotationIdx = i;
                }
            }
        }
        i++;
    }

    // get start annotation
    const Annotation *startAnnotation = NULL;
    if (annotationIdx < selectionData.size() && annotationIdx != -1) {
        startAnnotation = &selectionData[annotationIdx].annotation;
    }

    const QList<AnnotationTableObject *> items = annotatedDnaView->getAnnotationObjects(true);
    QList<Annotation> annotForNextPrev;
    if (as->isEmpty()) {
        // no selected annotations - choose first one
        searchNextPosition(items, -1, true, start);
        searchAnnotWithEqualsStartPos(items, annotForNextPrev, NULL, start);
        startAnnotation = &annotForNextPrev.first();
    }

    return startAnnotation;
}

void AnnotHighlightWidget::annotationNavigate(bool isForward) {
    qint64 startPos = -1;
    const Annotation *startAnnotation = findFirstSelectedAnnotationRegion(startPos, isForward);
    CHECK(startAnnotation != NULL, );

    // find annotations started at the same position
    const QList<AnnotationTableObject *> items = annotatedDnaView->getAnnotationObjects(true);
    QList<Annotation> annotationsInTheSamePosition;
    qint64 idx = searchAnnotWithEqualsStartPos(items, annotationsInTheSamePosition, startAnnotation, startPos);
    idx += 2 * isForward - 1;

    // check if next annotation is already selected
    AnnotationSelection* as = annotatedDnaView->getAnnotationsSelection();
    CHECK(NULL != as, );
    while (isValidIndex(annotationsInTheSamePosition, idx)) {
        if (!as->contains(annotationsInTheSamePosition[idx])) {
            break;
        }
        idx += 2 * isForward - 1;
    }

    int locIdx = 0;
    if (!isValidIndex(annotationsInTheSamePosition, idx)) {
        qint64 nextAnnPosition = -1;
        locIdx = searchNextPosition(items, startPos, isForward, nextAnnPosition);
        // there is no more annotations
        if (locIdx == -1) {
            return;
        }
        annotationsInTheSamePosition.clear();
        searchAnnotWithEqualsStartPos(items, annotationsInTheSamePosition, NULL, nextAnnPosition);
        if (annotationsInTheSamePosition.size() != 0) {
            idx = (isForward)?(0):(annotationsInTheSamePosition.size() - 1);
        } else {
            idx = -1;
        }
    }

    const Annotation *nextAnnotation = startAnnotation;
    if (isValidIndex(annotationsInTheSamePosition, idx)) {
        nextAnnotation = &annotationsInTheSamePosition[idx];
    }
    if (nextAnnotation != NULL && nextAnnotation != startAnnotation) {
        as->clear();
        as->addToSelection(*nextAnnotation, locIdx);
    }
}

void AnnotHighlightWidget::sl_onNextAnnotationClick() {
    annotationNavigate(true);
}

void AnnotHighlightWidget::sl_onPrevAnnotationClick() {
    annotationNavigate(false);
}

void AnnotHighlightWidget::sl_onAnnotationSelectionChanged() {
    AnnotationSelection* as = annotatedDnaView->getAnnotationsSelection();
    CHECK(as != NULL, );
    if (as->isEmpty()) {
        nextAnnotationButton->setDisabled(false);
        prevAnnotationButton->setDisabled(true);
    } else {
        nextAnnotationButton->setDisabled(false);
        prevAnnotationButton->setDisabled(false);

        // find first or last annotation region
        foreach (AnnotationSelectionData selData, as->getSelection()) {
            Annotation a = selData.annotation;
            foreach (U2Region region, selData.getSelectedRegions()) {
                if (isFirstAnnotationRegion(&a, region, false)) {
                    nextAnnotationButton->setDisabled(true);
                }
                if (isFirstAnnotationRegion(&a, region)) {
                    prevAnnotationButton->setDisabled(true);
                }
            }
        }

    }
}

void AnnotHighlightWidget::setNoAnnotTypesLabelValue()
{
    QList<ADVSequenceObjectContext*> seqContexts = annotatedDnaView->getSequenceContexts();

    if (1 == seqContexts.count()) {
        noAnnotTypesLabel->setText(tr("The sequence doesn't have any annotations."));
    }
    else {
        noAnnotTypesLabel->setText(tr("The sequences do not have any annotations."));
    }
}


void AnnotHighlightWidget::setNoAnnotsLayout()
{
    noAnnotTypesLabel->show();
    annotTreeTitle->hide();
    annotTree->hide();
    settingsTitle->hide();
    annotSettingsWidget->hide();
}


void AnnotHighlightWidget::setLayoutWithAnnotsSelection()
{
    noAnnotTypesLabel->hide();
    annotTreeTitle->show();
    annotTree->show();
    settingsTitle->show();
    annotSettingsWidget->show();
}


void AnnotHighlightWidget::connectSlots()
{
    // Show annotation types for the sequence or all annotation types
    connect(showAllLabel, SIGNAL(si_showAllStateChanged()), this, SLOT(sl_onShowAllStateChanged()));

    // Selected annotation type has changed
    connect(annotTree, SIGNAL(si_selectedItemChanged(QString)), SLOT(sl_onSelectedItemChanged(QString)));

    // Another color or a setting has been selected for an annotation type
    connect(annotTree, SIGNAL(si_colorChanged(QString, QColor)), SLOT(sl_storeNewColor(QString, QColor)));
    connect(annotSettingsWidget, SIGNAL(si_annotSettingsChanged(AnnotationSettings*)),
        SLOT(sl_storeNewSettings(AnnotationSettings*)));

    // A sequence has been modified (a subsequence added, removed, etc.)
    connect(annotatedDnaView, SIGNAL(si_sequenceModified(ADVSequenceObjectContext*)),
        this, SLOT(sl_onSequenceModified(ADVSequenceObjectContext*)));

    // An annotation object has been added/removed - connect/disconnect slots
    connect(annotatedDnaView, SIGNAL(si_annotationObjectAdded(AnnotationTableObject*)),
        SLOT(sl_onAnnotationObjectAdded(AnnotationTableObject*)));
    connect(annotatedDnaView, SIGNAL(si_annotationObjectRemoved(AnnotationTableObject*)),
        SLOT(sl_onAnnotationObjectRemoved(AnnotationTableObject*)));

    // An annotation has been added/removed/modified
    QList<AnnotationTableObject *> seqAnnotTableObjs = annotatedDnaView->getAnnotationObjects(true); // "true" to include auto-annotations
    foreach ( const AnnotationTableObject *annotTableObj, seqAnnotTableObjs ) {
        connectSlotsForAnnotTableObj(annotTableObj);
    }

    connect(prevAnnotationButton, SIGNAL(clicked()),this, SLOT(sl_onPrevAnnotationClick()));

    connect(nextAnnotationButton, SIGNAL(clicked()), this, SLOT(sl_onNextAnnotationClick()));

    AnnotationSelection* as = annotatedDnaView->getAnnotationsSelection();
    CHECK(as != NULL, );
    connect(as, SIGNAL(si_selectionChanged(AnnotationSelection*,QList<Annotation>,QList<Annotation>)),
            SLOT(sl_onAnnotationSelectionChanged()));
}

void AnnotHighlightWidget::connectSlotsForAnnotTableObj(const AnnotationTableObject *annotTableObj)
{
    connect(annotTableObj, SIGNAL(si_onAnnotationsAdded(const QList<Annotation> &)),
        SLOT(sl_onAnnotationsAddedRemoved(const QList<Annotation> &)));
    connect(annotTableObj, SIGNAL(si_onAnnotationsRemoved(const QList<Annotation> &)),
        SLOT(sl_onAnnotationsAddedRemoved(const QList<Annotation> &)));
    connect(annotTableObj, SIGNAL(si_onAnnotationModified(const AnnotationModification &)),
        SLOT(sl_onAnnotationModified(const AnnotationModification &)));
}

void AnnotHighlightWidget::disconnectSlotsForAnnotTableObj(const AnnotationTableObject *annotTableObj)
{
    disconnect(annotTableObj, SIGNAL(si_onAnnotationsAdded(const QList<Annotation> &)),
        this, SLOT(sl_onAnnotationsAddedRemoved(const QList<Annotation> &)));
    disconnect(annotTableObj, SIGNAL(si_onAnnotationsRemoved(const QList<Annotation> &)),
        this, SLOT(sl_onAnnotationsAddedRemoved(const QList<Annotation>&)));
    disconnect(annotTableObj, SIGNAL(si_onAnnotationModified(const AnnotationModification&)),
        this, SLOT(sl_onAnnotationModified(const AnnotationModification&)));
}

void AnnotHighlightWidget::sl_onShowAllStateChanged()
{
    loadAnnotTypes();
}


void AnnotHighlightWidget::sl_onSelectedItemChanged(const QString& annotName)
{
    AnnotationSettingsRegistry* annotRegistry = AppContext::getAnnotationsSettingsRegistry();

    AnnotationSettings* selectedAnnotSettings = annotRegistry->getAnnotationSettings(annotName);
    annotSettingsWidget->setSettings(selectedAnnotSettings, annotNamesWithAminoInfo.value(annotName));
}


void AnnotHighlightWidget::findAllAnnotationsNamesForSequence()
{
    annotNamesWithAminoInfo.clear();

    QList<ADVSequenceObjectContext*> seqObjContexts = annotatedDnaView->getSequenceContexts();

    foreach (ADVSequenceObjectContext* seqContext, seqObjContexts) {
        const DNAAlphabet* seqAlphabet = seqContext->getAlphabet();
        bool isAminoSeq = seqAlphabet->isAmino();

        QSet<AnnotationTableObject *> seqAnnotTableObjects = seqContext->getAnnotationObjects(true);

        foreach ( AnnotationTableObject *annotTableObj, seqAnnotTableObjects) {
            const QList<Annotation> annotations = annotTableObj->getAnnotations();

            foreach ( const Annotation &annot, annotations) {
                const QString annotName = annot.getName();

                // If the annotation was found on a nucleotide sequence
                if (!isAminoSeq) {
                    // The "Show on translation" option should be enabled for this annotation type
                    annotNamesWithAminoInfo.insert(annotName, false);
                }
                // The annotation was found on an amino acid sequence
                else {
                    if (annotNamesWithAminoInfo.contains(annotName)) {
                        // Do nothing:
                        // if the value is "true", then it doesn't make sense to rewrite the same value
                        // if the value is "false", then the annotation was previously found on a nucleotide
                        // sequence and therefore the "Show on translation" option should be enabled
                    }
                    else {
                        // Disable the option
                        annotNamesWithAminoInfo.insert(annotName, true);
                    }
                }
            }
        }
    }
}


void AnnotHighlightWidget::findAllAnnotationsNamesInSettings()
{
    annotNamesWithAminoInfo.clear();

    AnnotationSettingsRegistry* registry = AppContext::getAnnotationsSettingsRegistry();
    SAFE_POINT(0 != registry, "AnnotationSettingsRegistry is NULL!", );

    QStringList annotSettings = registry->getAllSettings();
    foreach (QString setting, annotSettings) {
        annotNamesWithAminoInfo.insert(setting, false);
    }
}

void AnnotHighlightWidget::loadAnnotTypes()
{
    // Get the annotation names
    if (showAllLabel->isShowAllSelected()) {
        findAllAnnotationsNamesInSettings();
    } else {
        findAllAnnotationsNamesForSequence();
    }

    QList<QString> annotNames = annotNamesWithAminoInfo.keys();
    qSort(annotNames);

    // Get the currently selected annotation name
    QString currentAnnotName = annotTree->getCurrentItemAnnotName();

    // Clear the old items and restore the initial height
    annotTree->clear();

    // If there types to show
    if (!annotNames.isEmpty()) {
        setLayoutWithAnnotsSelection();

        // Add the tree items
        AnnotationSettingsRegistry* annotRegistry = AppContext::getAnnotationsSettingsRegistry();
        foreach(const QString& name, annotNames) {
            AnnotationSettings* annotSettings = annotRegistry->getAnnotationSettings(name);
            annotTree->addItem(name, annotSettings->color);
        }

        // By default, select either previously selected item (if it is present) or the first item
        if (annotNames.contains(currentAnnotName)) {
            annotTree->setItemSelectedWithAnnotName(currentAnnotName);
        }
        else {
            annotTree->setFirstItemSelected();
            currentAnnotName = annotTree->getFirstItemAnnotName();
            SAFE_POINT(currentAnnotName != QString(), "Failed to get first annotation name!",);
        }

        // Set the configuration settings for the item
        AnnotationSettings* currentAnnotSettings = annotRegistry->getAnnotationSettings(currentAnnotName);
        annotSettingsWidget->setSettings(currentAnnotSettings, annotNamesWithAminoInfo.value(currentAnnotName));
    }
    else {
        setNoAnnotsLayout();
    }

    nextAnnotationButton->setDisabled( annotatedDnaView->getAnnotationObjects().isEmpty() );
}

void AnnotHighlightWidget::sl_storeNewColor(const QString& annotName, const QColor& newColor)
{
    QList<AnnotationSettings*> annotToWrite;
    AnnotationSettingsRegistry* annotRegistry = AppContext::getAnnotationsSettingsRegistry();
    AnnotationSettings* annotSettings = annotRegistry->getAnnotationSettings(annotName);
    if (annotSettings->color != newColor) {
        annotSettings->color = newColor;
        annotToWrite.append(annotSettings);
        annotRegistry->changeSettings(annotToWrite, true);
    }
}

void AnnotHighlightWidget::sl_storeNewSettings(AnnotationSettings* annotSettings)
{
    QList<AnnotationSettings*> annotToWrite;
    AnnotationSettingsRegistry* annotRegistry = AppContext::getAnnotationsSettingsRegistry();
    annotToWrite.append(annotSettings);
    annotRegistry->changeSettings(annotToWrite, true);
}

void AnnotHighlightWidget::sl_onSequenceModified(ADVSequenceObjectContext* /* seqContext */)
{
    loadAnnotTypes();
}

void AnnotHighlightWidget::sl_onAnnotationsAddedRemoved(const QList<Annotation>& /* annotations */)
{
    loadAnnotTypes();
}

void AnnotHighlightWidget::sl_onAnnotationModified(const AnnotationModification& /* modifications */)
{
    loadAnnotTypes();
}

void AnnotHighlightWidget::sl_onAnnotationObjectAdded(AnnotationTableObject *annotTableObj)
{
    connectSlotsForAnnotTableObj(annotTableObj);
    loadAnnotTypes();
}

void AnnotHighlightWidget::sl_onAnnotationObjectRemoved(AnnotationTableObject *annotTableObj)
{
    disconnectSlotsForAnnotTableObj(annotTableObj);
    loadAnnotTypes();
}

} // namespace
