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

#include "FindPatternTask.h"
#include "FindPatternWidget.h"

#include <U2Algorithm/FindAlgorithmTask.h>

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>


namespace U2 {

const static QString SHOW_OPTIONS_LINK("show_options_link");

const int FindPatternWidget::DEFAULT_RESULTS_NUM_LIMIT = 100000;
const int FindPatternWidget::DEFAULT_REGEXP_RESULT_LENGTH_LIMIT = 10000;


FindPatternWidget::FindPatternWidget(AnnotatedDNAView* _annotatedDnaView)
    : annotatedDnaView(_annotatedDnaView)
{
    setupUi(this);

    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    if (activeContext != 0) {
        // Initializing the annotation model
        CreateAnnotationModel annotModel;
        annotModel.hideLocation = true;
        annotModel.sequenceObjectRef = annotatedDnaView->getSequenceInFocus()->getSequenceObject();
        annotModel.sequenceLen = annotatedDnaView->getSequenceInFocus()->getSequenceLength();

        annotController = new CreateAnnotationWidgetController(annotModel, this, compact);

        setContentsMargins(0, 0, 0, 0);
        
        annotsWidget = annotController->getWidget();
        layoutAnnots->addWidget(annotsWidget);
        layoutAnnots->setSpacing(0);
        layoutAnnots->setContentsMargins(0, 0, 8, 0);

        DNAAlphabet* alphabet = activeContext->getAlphabet();
        isAminoSequenceSelected = alphabet->isAmino();

        initLayout();
        connectSlots();

        checkState();
    }
}


void FindPatternWidget::updateShowOptions()
{
    // Change the label
    QString linkText = showMore ? tr("Show more options") : tr("Show less options");
    linkText = QString("<a href=\"%1\" style=\"color: gray\">").arg(SHOW_OPTIONS_LINK)
        + linkText
        + QString("</a>");

    lblShowMoreLess->setText(linkText);

    // Show/hide the additional options
    if (showMore) {
        groupAlgorithm->hide();
        groupSearchIn->hide();
        groupOther->hide();
        annotsWidget->hide();
    }
    else {
        groupAlgorithm->show();
        groupSearchIn->show();
        groupOther->show();
        annotsWidget->show();
    }

    // Change the mode
    showMore = !showMore;
}


void FindPatternWidget::initLayout()
{
    // Hide all additional options
    showMore = true;
    updateShowOptions();

    initAlgorithmLayout();
    initStrandSelection();
    initSeqTranslSelection();
    initRegionSelection();
    initResultsLimit();

    lblErrorMessage->hide();
    lblErrorMessage->setStyleSheet(
        "color: red;"
        "font: bold;");

    updateLayout();
    
    layoutSearchButton->setAlignment(Qt::AlignTop);
    this->layout()->setAlignment(Qt::AlignTop);
}


void FindPatternWidget::initAlgorithmLayout()
{
    boxAlgorithm->addItem(tr("InsDel"), FindAlgorithmPatternSettings_InsDel);
    boxAlgorithm->addItem(tr("Substitute"), FindAlgorithmPatternSettings_Subst);
    boxAlgorithm->addItem(tr("Regular expression"), FindAlgorithmPatternSettings_RegExp);

    layoutMismatch = new QHBoxLayout(this);

    lblMatch = new QLabel(tr("Should match"));

    spinMatch = new QSpinBox(this);
    spinMatch->setSuffix("%"); // Percentage value
    spinMatch->setMinimum(30);
    spinMatch->setMaximum(100);
    spinMatch->setSingleStep(1);
    spinMatch->setValue(100);

    layoutMismatch->addWidget(lblMatch);
    layoutMismatch->addWidget(spinMatch);

    layoutAlgorithmSettings->addLayout(layoutMismatch);

    useAmbiguousBasesBox = new QCheckBox(tr("Search with ambiguous bases"));
    layoutAlgorithmSettings->addWidget(useAmbiguousBasesBox);

    layoutRegExpLen = new QHBoxLayout();
    boxUseMaxResultLen = new QCheckBox(tr("Limit result length"));
    boxMaxResultLen = new QSpinBox();
    boxMaxResultLen->setMinimum(0);
    boxMaxResultLen->setMaximum(500);
    boxMaxResultLen->setSingleStep(20);
    layoutRegExpLen->addWidget(boxUseMaxResultLen);
    layoutRegExpLen->addWidget(boxMaxResultLen);
    layoutAlgorithmSettings->addLayout(layoutRegExpLen);

    selectedAlgorithm = FindAlgorithmPatternSettings_InsDel;
}


void FindPatternWidget::initStrandSelection()
{
    boxStrand->addItem(tr("Both"), FindAlgorithmStrand_Both);
    boxStrand->addItem(tr("Direct"), FindAlgorithmStrand_Direct);
    boxStrand->addItem(tr("Reverse-coml."), FindAlgorithmStrand_Complement);
}


void FindPatternWidget::initSeqTranslSelection()
{
    boxSeqTransl->addItem(tr("Sequence"), SeqTranslIndex_Sequence);
    boxSeqTransl->addItem(tr("Translation"), SeqTranslIndex_Translation);
}


void FindPatternWidget::initRegionSelection()
{
    boxRegion->addItem(tr("Whole sequence"), RegionSelectionIndex_WholeSequence);
    boxRegion->addItem(tr("Custom region"), RegionSelectionIndex_CustomRegion);

    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != activeContext, "Internal error: sequence context is NULL during region selection init.",);

    setRegionToWholeSequence();

    editStart->setValidator(new QIntValidator(1, activeContext->getSequenceLength(), editStart));
    editEnd->setValidator(new QIntValidator(1, activeContext->getSequenceLength(), editEnd));

    sl_onRegionOptionChanged(RegionSelectionIndex_WholeSequence);
}


void FindPatternWidget::initResultsLimit()
{
    boxMaxResult->setMinimum(0);
    boxMaxResult->setMaximum(INT_MAX);
    boxMaxResult->setValue(DEFAULT_RESULTS_NUM_LIMIT);
    boxMaxResult->setEnabled(false);
}


void FindPatternWidget::connectSlots()
{
    connect(lblShowMoreLess, SIGNAL(linkActivated(const QString&)), SLOT(sl_showLessClicked(const QString&)));
    connect(boxAlgorithm, SIGNAL(currentIndexChanged(int)), SLOT(sl_onAlgorithmChanged(int)));
    connect(boxRegion, SIGNAL(currentIndexChanged(int)), SLOT(sl_onRegionOptionChanged(int)));
    connect(textPattern, SIGNAL(textChanged()), SLOT(sl_onSearchPatternChanged()));
    connect(boxUseMaxResult, SIGNAL(stateChanged(int)), SLOT(sl_enableBoxMaxResult(int)));
    connect(editStart, SIGNAL(textEdited(QString)), SLOT(sl_onRegionValueEdited()));
    connect(editEnd, SIGNAL(textEdited(QString)), SLOT(sl_onRegionValueEdited()));
    connect(spinMatch, SIGNAL(valueChanged()), SLOT(sl_onMatchPercentChanged()));
    connect(boxSeqTransl, SIGNAL(currentIndexChanged(int)), SLOT(sl_onSequenceTranslationChanged(int)));
    connect(btnSearch, SIGNAL(clicked()), SLOT(sl_onSearchClicked()));

    // A sequence has been selected in the Sequence View
    connect(annotatedDnaView, SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)),
        this, SLOT(sl_onFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*)));

    // A sequence has been modified (a subsequence added, removed, etc.)
    connect(annotatedDnaView, SIGNAL(si_sequenceModified(ADVSequenceObjectContext*)),
        this, SLOT(sl_onSequenceModified(ADVSequenceObjectContext*)));
}


void FindPatternWidget::sl_enableBoxMaxResult(int checkBoxState)
{
    if (Qt::Checked == checkBoxState) {
        boxMaxResult->setEnabled(true);
    }
    else {
        boxMaxResult->setValue(DEFAULT_RESULTS_NUM_LIMIT);
        boxMaxResult->setEnabled(false);
    }
}


void FindPatternWidget::sl_showLessClicked(const QString& link)
{
    SAFE_POINT(SHOW_OPTIONS_LINK == link, "Incorrect link!",);
    updateShowOptions();
}


void FindPatternWidget::sl_onAlgorithmChanged(int index)
{
    selectedAlgorithm = boxAlgorithm->itemData(index).toInt();
    updateLayout();
}


void FindPatternWidget::sl_onRegionOptionChanged(int index)
{
    if (boxRegion->itemData(index).toInt() == RegionSelectionIndex_WholeSequence) {
        editStart->hide();
        lblStartEndConnection->hide();
        editEnd->hide();
    }
    else if (boxRegion->itemData(index).toInt() == RegionSelectionIndex_CustomRegion) {
        editStart->show();
        lblStartEndConnection->show();
        editEnd->show();
    }
}


void highlightBackground(QWidget* widget)
{
    widget->setStyleSheet("background-color: " + L10N::redLightColorStr() + ";");
}


void doNotHighlightBackground(QWidget* widget)
{
    widget->setStyleSheet("background-color: white;");
}


void FindPatternWidget::sl_onRegionValueEdited()
{
    regionIsCorrect = true;

    // The values are not empty
    if (editStart->text().isEmpty()) {
        highlightBackground(editStart);
        regionIsCorrect = false;
    }
    else if (editEnd->text().isEmpty()) {
        highlightBackground(editEnd);
        regionIsCorrect = false;
    }
    else {
        // Start <= end
        bool ok = false;
        qint64 value1 = editStart->text().toLongLong(&ok);
        if (!ok || (value1 < 1)) {
            highlightBackground(editStart);
            regionIsCorrect = false;
        }

        int value2 = editEnd->text().toLongLong(&ok);
        if (!ok) {
            highlightBackground(editEnd);
            regionIsCorrect = false;
        }

        if (ok && (value1 > value2)) {
            highlightBackground(editStart);
            highlightBackground(editEnd);
            regionIsCorrect = false;
        }
    }

    if (regionIsCorrect) {
        doNotHighlightBackground(editStart);
        doNotHighlightBackground(editEnd);
    }

    checkState();
}


void FindPatternWidget::sl_onMatchPercentChanged()
{
    tunePercentBox();
}


void FindPatternWidget::sl_onFocusChanged(
    ADVSequenceWidget* /* prevWidget */, ADVSequenceWidget* currentWidget)
{
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    if (activeContext != 0) {
        DNAAlphabet* alphabet = activeContext->getAlphabet();
        isAminoSequenceSelected = alphabet->isAmino();
        updateLayout();

        // Update region
        setRegionToWholeSequence();
        doNotHighlightBackground(editStart);
        doNotHighlightBackground(editEnd);

        // Update available annotations table objects, etc.
        updateAnnotationsWidget();
    }
}


void FindPatternWidget::updateLayout()
{
    // Algorithm group
    if (selectedAlgorithm == FindAlgorithmPatternSettings_InsDel) {
        useAmbiguousBasesBox->setChecked(false);
        useAmbiguousBasesBox->hide();
        boxUseMaxResultLen->hide();
        boxMaxResultLen->hide();
        enableDisableMatchSpin();
    }
    else if (selectedAlgorithm == FindAlgorithmPatternSettings_Subst) {
        useAmbiguousBasesBox->show();
        boxUseMaxResultLen->hide();
        boxMaxResultLen->hide();
        enableDisableMatchSpin();
    }
    else if (selectedAlgorithm == FindAlgorithmPatternSettings_RegExp) {
        useAmbiguousBasesBox->setChecked(false);
        useAmbiguousBasesBox->hide();
        boxUseMaxResultLen->show();
        boxMaxResultLen->show();
        spinMatch->hide();
        lblMatch->hide();
    }

    // "Search in" group
    if (isAminoSequenceSelected) {
        lblStrand->hide();
        boxStrand->hide();
        lblSeqTransl->hide();
        boxSeqTransl->hide();
    }
    else {
        lblStrand->show();
        boxStrand->show();
        lblSeqTransl->show();
        boxSeqTransl->show();
    }
}

void FindPatternWidget::showHideErrorMessage(bool show, ErrorMessageFlag errorMessageFlag)
{
    if (show) {
        if (!errorFlags.contains(errorMessageFlag)) {
            errorFlags.append(errorMessageFlag);
        }
    }
    else {
        errorFlags.removeAll(errorMessageFlag);
    }

    if (!errorFlags.isEmpty()) {
        QString text = "";
        foreach (ErrorMessageFlag flag, errorFlags) {
            switch (flag) {
                case PatternIsTooLong:
                    if (!text.isEmpty()) {
                        text += "\n";
                    }
                    text += QString(tr("The value is longer than the search region."
                        " Please input a shorter value or select another region!"));
                    break;
                case PatternAlphabetDoNotMatch:
                    if (!text.isEmpty()) {
                        text += "\n";
                    }
                    text += QString(tr("Warning: input value contains characters that"
                        " do not match the active alphabet!"));
                    break;
                default:
                    FAIL("Unexpected value of the error flag in show/hide error message for pattern!",);
            }
        }
        lblErrorMessage->setText(text);
        lblErrorMessage->show();
    }
    else {
        lblErrorMessage->hide();
        lblErrorMessage->setText("");
    }
}


void FindPatternWidget::sl_onSearchPatternChanged()
{
    checkState();
    tunePercentBox();
    enableDisableMatchSpin();

    // Show a warning if the pattern alphabet doesn't match,
    // but do not block the "Search" button
    verifyPatternAlphabet();
}


void FindPatternWidget::setRegionToWholeSequence()
{
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != activeContext, "Internal error: sequence context is NULL during setting region to whole sequence.",);

    editStart->setText(QString::number(1));
    editEnd->setText(QString::number(activeContext->getSequenceLength()));
    regionIsCorrect = true;
}


void FindPatternWidget::verifyPatternAlphabet()
{
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != activeContext, "Internal error: there is no sequence in focus on pattern search!",);

    DNAAlphabet* alphabet = activeContext->getAlphabet();
    if (!isAminoSequenceSelected && SeqTranslIndex_Translation == boxSeqTransl->currentIndex()) {
        DNATranslation* translation = activeContext->getAminoTT();
        SAFE_POINT(NULL != translation, "Failed to get translation on pattern search!",);

        alphabet = translation->getDstAlphabet();
    }

    QString pattern = textPattern->toPlainText();
    pattern = pattern.toUpper();

    bool alphabetIsOk =
        (TextUtils::fits(alphabet->getMap(),
        pattern.toLocal8Bit().data(),
        pattern.size()))    ||
        (useAmbiguousBasesBox->isChecked())   ||
        (FindAlgorithmPatternSettings_RegExp == selectedAlgorithm);

    if (!alphabetIsOk) {
        showHideErrorMessage(true, PatternAlphabetDoNotMatch);
    }
    else {
        showHideErrorMessage(false, PatternAlphabetDoNotMatch);
    }
}



void FindPatternWidget::sl_onSequenceTranslationChanged(int /* index */)
{
    verifyPatternAlphabet();
}


void FindPatternWidget::sl_onSequenceModified(ADVSequenceObjectContext* /* context */)
{
    setRegionToWholeSequence();
    checkState();
    verifyPatternAlphabet();
}


void FindPatternWidget::checkState()
{
    // Disable the "Search" button if the pattern is empty
    if (textPattern->toPlainText().isEmpty()) {
        btnSearch->setDisabled(true);
        return;
    }

    // Disable if the region is not correct
    if (!regionIsCorrect) {
        btnSearch->setDisabled(true);
        return;
    }

    // Disable if the length of the pattern is greater than the search region length
    int maxError = getMaxError();
    qint64 patternLength = textPattern->toPlainText().length();
    qint64 minMatch = patternLength - maxError;
    SAFE_POINT(minMatch > 0, "Search pattern length is greater than max error value!",);

    qint64 regionLength = editEnd->text().toLongLong() - editStart->text().toLongLong() + 1;
    SAFE_POINT(regionLength > 0, "Incorrect region length when enabling/disabling the pattern search button.",);

    if (minMatch > regionLength) {
        btnSearch->setDisabled(true);
        highlightBackground(textPattern);
        showHideErrorMessage(true, PatternIsTooLong);
        return;
    }
    else {
        doNotHighlightBackground(textPattern);
        showHideErrorMessage(false, PatternIsTooLong);
    }

    // Otherwise enable the button
    btnSearch->setDisabled(false);
}


void FindPatternWidget::enableDisableMatchSpin()
{
    if (textPattern->toPlainText().isEmpty() || isAminoSequenceSelected) {
        spinMatch->setEnabled(false);
    }
    else {
        spinMatch->setEnabled(true);
    }
}


U2Region FindPatternWidget::getCompleteSearchRegion(bool& regionIsCorrect, qint64 maxLen) const
{
    bool ok = false;
    qint64 value1 = editStart->text().toLongLong(&ok) - 1;
    if (!ok || value1 < 0) {
        regionIsCorrect = false;
        return U2Region();
    }

    int value2 = editEnd->text().toLongLong(&ok);
    if (!ok || value2 <= 0 || value2 > maxLen){
        regionIsCorrect = false;
        return U2Region();
    }

    if (value1 > value2 ) { // start > end
        regionIsCorrect = false;
        return U2Region();
    }

    regionIsCorrect = true;
    return U2Region(value1, value2 - value1);
}


int FindPatternWidget::getMaxError() const
{
    return int((float)(1 - float(spinMatch->value()) / 100) * textPattern->toPlainText().length());
}


void FindPatternWidget::tunePercentBox()
{
    int patternLength = qMax(1, textPattern->toPlainText().length());
    int percentage = spinMatch->value();
    int step = qMax(1, 100 / patternLength);
    spinMatch->setSingleStep(step);

    int diff = percentage % step;
    if (diff == 0 || percentage == 100) {
        return;
    }

    int newValue = percentage;
    if (diff > step / 2) {
        newValue = qMin(100, newValue + (step - diff));
    }
    else {
        newValue -= step;
    }
    SAFE_POINT(newValue <= 100,
        QString("Internal error: unexpected value during tuning of the match percentage value '%1.'").arg(newValue),);

    spinMatch->setValue(newValue);
}


void FindPatternWidget::sl_onSearchClicked()
{
    SAFE_POINT(!textPattern->toPlainText().isEmpty(), "Internal error: can't search for an empty string!",);

    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != activeContext, "Internal error: there is no sequence in focus!",);

    FindAlgorithmTaskSettings settings;
    settings.sequence = activeContext->getSequenceObject()->getWholeSequenceData();
    settings.pattern = textPattern->toPlainText().toLocal8Bit();

    // Strand
    if (isAminoSequenceSelected) {
        settings.strand = FindAlgorithmStrand_Direct;
    }
    else {
        int strandId = boxStrand->currentIndex();
        switch (strandId) {
            case FindAlgorithmStrand_Both:
                settings.strand = FindAlgorithmStrand_Both;
                break;
            case FindAlgorithmStrand_Direct:
                settings.strand = FindAlgorithmStrand_Direct;
                break;
            case FindAlgorithmStrand_Complement:
                settings.strand = FindAlgorithmStrand_Complement;
                break;
            default:
                FAIL("Unexpected value of the strand parameter!",);
        }
    }

    if (!isAminoSequenceSelected) {
        settings.complementTT = activeContext->getComplementTT();
        if (NULL == settings.complementTT && settings.strand != FindAlgorithmStrand_Direct) {
            coreLog.error(tr("Unable to search on the reverse-complement strand,"
                " searching on the direct strand only!"));
            settings.strand = FindAlgorithmStrand_Direct;
        }
    }

    // Sequence/translation
    if (!isAminoSequenceSelected && (SeqTranslIndex_Translation == boxSeqTransl->currentIndex())) {
        settings.proteinTT = activeContext->getAminoTT();
    }
    else {
        settings.proteinTT = NULL;
    }

    // Limit results number to the specified value
    settings.maxResult2Find = boxUseMaxResult->isChecked() ?
        boxMaxResult->value() :
        DEFAULT_RESULTS_NUM_LIMIT;

    // Region
    bool regionIsCorrectRef = false;
    U2Region region = getCompleteSearchRegion(regionIsCorrectRef, activeContext->getSequenceLength());
    SAFE_POINT(true == regionIsCorrectRef, "Internal error: incorrect search region has been supplied."
        " Skipping the pattern search.", );
    settings.searchRegion = region;

    // Not a single shot
    settings.singleShot = false;

    // Algorithm settings
    settings.patternSettings = static_cast<FindAlgorithmPatternSettings>(selectedAlgorithm);
    settings.maxErr = getMaxError();
    settings.useAmbiguousBases = useAmbiguousBasesBox->isChecked();
    settings.maxRegExpResult = boxUseMaxResultLen->isChecked() ?
        boxMaxResultLen->value() :
        DEFAULT_REGEXP_RESULT_LENGTH_LIMIT;

    // Preparing the annotations object and other annotations parameters
    annotController->prepareAnnotationObject();
    const CreateAnnotationModel& annotModel = annotController->getModel();
    QString annotName = annotModel.data->name;
    QString annotGroup = annotModel.groupName;

    // Creating and registering the task
    bool removeOverlaps = removeOverlapsBox->isChecked();
    FindPatternTask* task = new FindPatternTask(settings,
        annotModel.getAnnotationObject(),
        annotName,
        annotGroup,
        removeOverlaps);

    AppContext::getTaskScheduler()->registerTopLevelTask(task);

    updateAnnotationsWidget();
}


void FindPatternWidget::updateAnnotationsWidget()
{
    // Updating the annotations widget
    SAFE_POINT(NULL != annotatedDnaView->getSequenceInFocus(),
        "There is no sequence in focus to update the annotations widget on the 'Search in Sequence' tab.",);
    CreateAnnotationModel newAnnotModel;
    newAnnotModel.hideLocation = true;
    newAnnotModel.sequenceObjectRef = annotatedDnaView->getSequenceInFocus()->getSequenceObject();
    newAnnotModel.sequenceLen = annotatedDnaView->getSequenceInFocus()->getSequenceLength();
    annotController->updateWidgetForAnnotationModel(newAnnotModel);
}


} // namespace
