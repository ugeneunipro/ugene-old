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

#include "AnnotHighlightWidget.h"

#include <U2Core/AnnotationSettings.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2SafePoints.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>


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
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);

    noAnnotTypesLabel = new QLabel("");
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

    // Init main layout
    mainLayout->addWidget(noAnnotTypesLabel);
    mainLayout->addWidget(annotTreeTitle);
    mainLayout->addLayout(treeLayout);
    mainLayout->addLayout(settingsLayout);

    setLayout(mainLayout);
}


void AnnotHighlightWidget::setNoAnnotTypesLabelValue()
{
    QList<ADVSequenceObjectContext*> seqContexts = annotatedDnaView->getSequenceContexts();

    if (1 == seqContexts.count()) {
        noAnnotTypesLabel->setText(tr("The sequence doesn't have any\nannotations."));
    }
    else {
        noAnnotTypesLabel->setText(tr("The sequences do not have any\nannotations."));
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
    QList<AnnotationTableObject*> seqAnnotTableObjs = annotatedDnaView->getAnnotationObjects(true); // "true" to include auto-annotations
    foreach (AnnotationTableObject* annotTableObj, seqAnnotTableObjs) {
        connectSlotsForAnnotTableObj(annotTableObj);
    }
}

void AnnotHighlightWidget::connectSlotsForAnnotTableObj(const AnnotationTableObject* annotTableObj)
{
    connect(annotTableObj, SIGNAL(si_onAnnotationsAdded(const QList<Annotation*>&)),
        SLOT(sl_onAnnotationsAddedRemoved(const QList<Annotation*>&)));
    connect(annotTableObj, SIGNAL(si_onAnnotationsRemoved(const QList<Annotation*>&)),
        SLOT(sl_onAnnotationsAddedRemoved(const QList<Annotation*>&)));
    connect(annotTableObj, SIGNAL(si_onAnnotationModified(const AnnotationModification&)),
        SLOT(sl_onAnnotationModified(const AnnotationModification&)));
}


void AnnotHighlightWidget::disconnectSlotsForAnnotTableObj(const AnnotationTableObject* annotTableObj)
{
    disconnect(annotTableObj, SIGNAL(si_onAnnotationsAdded(const QList<Annotation*>&)),
        this, SLOT(sl_onAnnotationsAddedRemoved(const QList<Annotation*>&)));
    disconnect(annotTableObj, SIGNAL(si_onAnnotationsRemoved(const QList<Annotation*>&)),
        this, SLOT(sl_onAnnotationsAddedRemoved(const QList<Annotation*>&)));
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

        QSet<AnnotationTableObject*> seqAnnotTableObjects = seqContext->getAnnotationObjects(true);

        foreach (AnnotationTableObject* annotTableObj, seqAnnotTableObjects) {
            QList<Annotation*> annotations = annotTableObj->getAnnotations();

            foreach (Annotation* annot, annotations) {
                SAFE_POINT(0 != annot, "Annotation is NULL!", );
                QString annotName = annot->getAnnotationName();

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


void AnnotHighlightWidget::sl_onAnnotationsAddedRemoved(const QList<Annotation*>& /* annotations */)
{
    loadAnnotTypes();
}


void AnnotHighlightWidget::sl_onAnnotationModified(const AnnotationModification& /* modifications */)
{
    loadAnnotTypes();
}


void AnnotHighlightWidget::sl_onAnnotationObjectAdded(AnnotationTableObject* annotTableObj)
{
    connectSlotsForAnnotTableObj(annotTableObj);
    loadAnnotTypes();
}


void AnnotHighlightWidget::sl_onAnnotationObjectRemoved(AnnotationTableObject* annotTableObj)
{
    disconnectSlotsForAnnotTableObj(annotTableObj);
    loadAnnotTypes();
}


} // namespace
