/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QFlags>
#include <QKeyEvent>

#include <U2Algorithm/FindAlgorithmTask.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/FastaFormat.h>
#include <U2Formats/GenbankFeatures.h>

#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Gui/U2FileDialog.h>
#include <U2Gui/U2WidgetStateStorage.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVSequenceWidget.h>
#include <U2View/AnnotatedDNAView.h>

#include "FindPatternWidget.h"

const QString NEW_LINE_SYMBOL = "\n";
const QString STYLESHEET_COLOR_DEFINITION = "color: ";
const QString STYLESHEET_DEFINITIONS_SEPARATOR = ";";

const int REG_EXP_MIN_RESULT_LEN = 1;
const int REG_EXP_MAX_RESULT_LEN = 1000;
const int REG_EXP_MAX_RESULT_SINGLE_STEP = 20;

namespace U2 {

class FastaPatternsWalker {
public:
    FastaPatternsWalker(const QString &_patternsString, int _cursor = 0)
        : patternsString(_patternsString.toLatin1()), cursor(_cursor)
    {
        current = -1;
        header = false;
        comment = false;
    }

    bool hasNext() const {
        return (current < patternsString.size() - 1);
    }

    char next() {
        if (!hasNext()) {
            return 0;
        }
        current++;
        if (!updateMetaStart()) {
            if (isMetaChars() && '\n' == patternsString[current]) {
                header = false;
                comment = false;
            }
        }
        return patternsString[current];
    }

    bool isSequenceChar() const {
        CHECK(-1 != current, false);
        CHECK(current < patternsString.size(), false);
        return !isMetaChars();
    }

    bool isHeader() const {
        return header;
    }

    bool isComment() const {
        return comment;
    }

    /** moves current place to the previous */
    void removeCurrent() {
        CHECK(-1 != current, );
        CHECK(current < patternsString.size(), );
        patternsString.remove(current, 1);
        if (current < cursor) {
            cursor--;
        }
        current--;
    }

    bool isCorrect() const {
        if (!isSequenceChar()) {
            return true;
        }
        QChar c(patternsString[current]);
        if (c.isLetter()) {
            return c.isUpper();
        } else {
            return ('\n' == c);
        }
    }

    void setCurrent(char value) {
        CHECK(-1 != current, );
        CHECK(current < patternsString.size(), );
        patternsString[current] = value;
    }

    int getCursor() const {
        return cursor;
    }

    QString getString() const {
        return patternsString;
    }

private:
    QByteArray patternsString;
    int cursor;
    int current;
    bool comment;
    bool header;

private:
    bool updateMetaStart() {
        char c = patternsString[current];
        if (FastaFormat::FASTA_COMMENT_START_SYMBOL != c &&
            FastaFormat::FASTA_HEADER_START_SYMBOL != c) {
            return false;
        }
        if ((0 == current) || ('\n' == patternsString[current-1])) {
            comment = (FastaFormat::FASTA_COMMENT_START_SYMBOL == c);
            header = (FastaFormat::FASTA_HEADER_START_SYMBOL == c);
            return true;
        }
        return false;
    }
    bool isMetaChars() const {
        return header || comment;
    }
};

FindPatternEventFilter::FindPatternEventFilter(QObject* parent)
    : QObject(parent)
{
}


bool FindPatternEventFilter::eventFilter(QObject* obj, QEvent* event) {
    const QEvent::Type eventType = event->type();
    if (QEvent::KeyPress == eventType) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (Qt::Key_Enter == keyEvent->key() || Qt::Key_Return == keyEvent->key()) {
            if(keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
                emit si_shiftEnterPressed();
                return true;
            }else if(!keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                emit si_enterPressed();
                return true;
            }else{
                keyEvent->setModifiers(keyEvent->modifiers() & ~Qt::ControlModifier);
                return false;
            }
        }
    } else if (QEvent::Show == eventType) {
        QWidget *watched = dynamic_cast<QWidget *>(obj);
        if (NULL != watched) {
            watched->setFocus();
        }
    }
    return QObject::eventFilter(obj, event);
}



const static QString SHOW_OPTIONS_LINK("show_options_link");

const int FindPatternWidget::DEFAULT_RESULTS_NUM_LIMIT = 100000;
const int FindPatternWidget::DEFAULT_REGEXP_RESULT_LENGTH_LIMIT = 10000;

const QString FindPatternWidget::ALGORITHM_SETTINGS = QObject::tr("Search algorithm");
const QString FindPatternWidget::SEARCH_IN_SETTINGS = QObject::tr("Search in");
const QString FindPatternWidget::OTHER_SETTINGS = QObject::tr("Other settings");

FindPatternWidget::FindPatternWidget(AnnotatedDNAView* _annotatedDnaView)
    : annotatedDnaView(_annotatedDnaView), iterPos(1), searchTask(NULL), previousPatternString(""),
    savableWidget(this, GObjectViewUtils::findViewByName(_annotatedDnaView->getName()))
{
    setupUi(this);

    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    if (activeContext != 0) {
        // Initializing the annotation model
        CreateAnnotationModel annotModel;
        annotModel.hideLocation = true;
        annotModel.sequenceObjectRef = annotatedDnaView->getSequenceInFocus()->getSequenceObject();
        annotModel.sequenceLen = annotatedDnaView->getSequenceInFocus()->getSequenceLength();

        annotController = new CreateAnnotationWidgetController(annotModel, this, optPanel);
        annotModelPrepared = false;
        connect(annotController, SIGNAL(si_annotationNamesEdited()), SLOT(sl_onAnotationNameEdited()));

        setContentsMargins(0, 0, 0, 0);

        annotsWidget = annotController->getWidget();
        annotsWidget->setObjectName("annotationsWidget");
        usePatternNamesCheckBox = annotController->getUsePatternNameCheckBox();
        annotController->setUsePatternNameCheckBoxVisible();

        const DNAAlphabet* alphabet = activeContext->getAlphabet();
        isAminoSequenceSelected = alphabet->isAmino();

        initLayout();
        connectSlots();

        checkState();

        FindPatternEventFilter *findPatternEventFilter = new FindPatternEventFilter(this);
        textPattern->installEventFilter(findPatternEventFilter);

        setFocusProxy(textPattern);

        currentSelection = NULL;
        showHideMessage(true, UseMultiplePatternsTip);

        connect(findPatternEventFilter, SIGNAL(si_enterPressed()), SLOT(sl_onEnterPressed()));
        connect(findPatternEventFilter, SIGNAL(si_shiftEnterPressed()), SLOT(sl_onShiftEnterPressed()));
        connect(usePatternNamesCheckBox, SIGNAL(stateChanged(int)), SLOT(sl_usePatternNamesCbClicked()));

        sl_onSearchPatternChanged();
    }
    nextPushButton->setDisabled(true);
    prevPushButton->setDisabled(true);
    getAnnotationsPushButton->setDisabled(true);
    resultLabel->setText(tr("Results: 0/0"));
    setUpTabOrder();
    U2WidgetStateStorage::restoreWidgetState(savableWidget);
}

void FindPatternWidget::initLayout() {
    lblErrorMessage->setStyleSheet(
        "color: " + L10N::errorColorLabelStr() + ";"
        "font: bold;");
    lblErrorMessage->setText("");
    initAlgorithmLayout();
    initStrandSelection();
    initSeqTranslSelection();
    initRegionSelection();
    initResultsLimit();

    subgroupsLayout->setSpacing(0);
    subgroupsLayout->addWidget(new ShowHideSubgroupWidget(ALGORITHM_SETTINGS, ALGORITHM_SETTINGS, widgetAlgorithm, false));
    subgroupsLayout->addWidget(new ShowHideSubgroupWidget(SEARCH_IN_SETTINGS, SEARCH_IN_SETTINGS, widgetSearchIn, false));
    subgroupsLayout->addWidget(new ShowHideSubgroupWidget(OTHER_SETTINGS, OTHER_SETTINGS, widgetOther, false));
    subgroupsLayout->addWidget(annotsWidget);

    updateLayout();

    layoutSearchButton->setAlignment(Qt::AlignTop);
    this->layout()->setAlignment(Qt::AlignTop);

    this->layout()->setMargin(0);
}


void FindPatternWidget::initAlgorithmLayout()
{
    boxAlgorithm->addItem(tr("Exact"), FindAlgorithmPatternSettings_Exact);
    if(!isAminoSequenceSelected) {
        boxAlgorithm->addItem(tr("InsDel"), FindAlgorithmPatternSettings_InsDel);
        boxAlgorithm->addItem(tr("Substitute"), FindAlgorithmPatternSettings_Subst);
    }
    boxAlgorithm->addItem(tr("Regular expression"), FindAlgorithmPatternSettings_RegExp);

    layoutMismatch = new QHBoxLayout();

    lblMatch = new QLabel(tr("Should match"));

    spinMatch = new QSpinBox(this);
    spinMatch->setSuffix("%"); // Percentage value
    spinMatch->setMinimum(30);
    spinMatch->setMaximum(100);
    spinMatch->setSingleStep(1);
    spinMatch->setValue(100);
    spinMatch->setObjectName("spinBoxMatch");

    layoutMismatch->addWidget(lblMatch);
    layoutMismatch->addWidget(spinMatch);

    layoutAlgorithmSettings->addLayout(layoutMismatch);

    initUseAmbiguousBasesContainer();
    initMaxResultLenContainer();

    selectedAlgorithm = boxAlgorithm->itemData(boxAlgorithm->currentIndex()).toInt();
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
    boxRegion->addItem(FindPatternWidget::tr("Whole sequence"), RegionSelectionIndex_WholeSequence);
    boxRegion->addItem(FindPatternWidget::tr("Custom region"), RegionSelectionIndex_CustomRegion);
    boxRegion->addItem(FindPatternWidget::tr("Selected region"), RegionSelectionIndex_CurrentSelectedRegion);

    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != activeContext, "Internal error: sequence context is NULL during region selection init.",);

    setRegionToWholeSequence();

    editStart->setValidator(new QIntValidator(1, activeContext->getSequenceLength(), editStart));
    editEnd->setValidator(new QIntValidator(1, activeContext->getSequenceLength(), editEnd));

    currentSelection = annotatedDnaView->getSequenceInFocus()->getSequenceSelection();

    sl_onRegionOptionChanged(RegionSelectionIndex_WholeSequence);
}


void FindPatternWidget::initResultsLimit()
{
    boxMaxResult->setMinimum(1);
    boxMaxResult->setMaximum(INT_MAX);
    boxMaxResult->setValue(DEFAULT_RESULTS_NUM_LIMIT);
    boxMaxResult->setEnabled(true);
}

void FindPatternWidget::initUseAmbiguousBasesContainer() {
    useAmbiguousBasesContainer = new QWidget();

    QHBoxLayout *useAmbiguousBasesLayout = new QHBoxLayout();
    useAmbiguousBasesLayout->setContentsMargins(0, 0, 0, 0);
    useAmbiguousBasesLayout->setSpacing(10);
    useAmbiguousBasesLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    useAmbiguousBasesContainer->setLayout(useAmbiguousBasesLayout);

    useAmbiguousBasesBox = new QCheckBox();
    useAmbiguousBasesBox->setObjectName("useAmbiguousBasesBox");
    QLabel *useAmbiguousBasesLabel = new QLabel(tr("Search with ambiguous bases"));
    useAmbiguousBasesLabel->setWordWrap(true);

    useAmbiguousBasesLayout->addWidget(useAmbiguousBasesBox, 0);
    useAmbiguousBasesLayout->addWidget(useAmbiguousBasesLabel, 1);
    layoutAlgorithmSettings->addWidget(useAmbiguousBasesContainer);
}

void FindPatternWidget::initMaxResultLenContainer() {
    useMaxResultLenContainer = new QWidget();

    layoutRegExpLen = new QVBoxLayout();
    layoutRegExpLen->setContentsMargins(0, 0, 0, 0);
    layoutRegExpLen->setSpacing(3);
    layoutRegExpLen->setSizeConstraint(QLayout::SetMinAndMaxSize);
    useMaxResultLenContainer->setLayout(layoutRegExpLen);

    QHBoxLayout *layoutUseMaxResultLen = new QHBoxLayout();
    layoutUseMaxResultLen->setSpacing(10);
    layoutUseMaxResultLen->setSizeConstraint(QLayout::SetMinAndMaxSize);

    boxUseMaxResultLen = new QCheckBox();
    boxUseMaxResultLen->setObjectName("boxUseMaxResultLen");
    QLabel *labelUseMaxResultLen = new QLabel(tr("Results no longer than:"));
    labelUseMaxResultLen->setWordWrap(true);
    layoutUseMaxResultLen->addWidget(boxUseMaxResultLen, 0);
    layoutUseMaxResultLen->addWidget(labelUseMaxResultLen, 1);

    boxMaxResultLen = new QSpinBox();
    boxMaxResultLen->setObjectName("boxMaxResultLen");
    boxMaxResultLen->setMinimum(REG_EXP_MIN_RESULT_LEN);
    boxMaxResultLen->setMaximum(REG_EXP_MAX_RESULT_LEN);
    boxMaxResultLen->setSingleStep(REG_EXP_MAX_RESULT_SINGLE_STEP);
    boxMaxResultLen->setValue(REG_EXP_MAX_RESULT_LEN);
    boxMaxResultLen->setEnabled(false);
    connect(boxUseMaxResultLen, SIGNAL(toggled(bool)), boxMaxResultLen, SLOT(setEnabled(bool)));
    connect(boxUseMaxResultLen, SIGNAL(toggled(bool)), SLOT(sl_activateNewSearch()));
    connect(boxMaxResultLen, SIGNAL(valueChanged(int)), SLOT(sl_activateNewSearch()));

    layoutRegExpLen->addLayout(layoutUseMaxResultLen);
    layoutRegExpLen->addWidget(boxMaxResultLen);
    layoutAlgorithmSettings->addWidget(useMaxResultLenContainer);
}


void FindPatternWidget::connectSlots()
{
    connect(boxAlgorithm, SIGNAL(currentIndexChanged(int)), SLOT(sl_onAlgorithmChanged(int)));
    connect(boxRegion, SIGNAL(currentIndexChanged(int)), SLOT(sl_onRegionOptionChanged(int)));
    connect(textPattern, SIGNAL(textChanged()), SLOT(sl_onSearchPatternChanged()));
    connect(editStart, SIGNAL(textChanged(QString)), SLOT(sl_onRegionValueEdited()));
    connect(editEnd, SIGNAL(textChanged(QString)), SLOT(sl_onRegionValueEdited()));
    connect(boxSeqTransl, SIGNAL(currentIndexChanged(int)), SLOT(sl_onSequenceTranslationChanged(int)));

    connect(boxStrand, SIGNAL(currentIndexChanged(int)), SLOT(sl_activateNewSearch()));
    connect(boxSeqTransl, SIGNAL(currentIndexChanged(int)), SLOT(sl_activateNewSearch()));

    connect(removeOverlapsBox, SIGNAL(stateChanged(int)), SLOT(sl_activateNewSearch()));
    connect(boxMaxResult, SIGNAL(valueChanged(int)), SLOT(sl_activateNewSearch()));

    // A sequence has been selected in the Sequence View
    connect(annotatedDnaView, SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)),
        this, SLOT(sl_onFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*)));

    // A sequence has been modified (a subsequence added, removed, etc.)
    connect(annotatedDnaView->getSequenceInFocus()->getSequenceObject(), SIGNAL(si_sequenceChanged()),
        this, SLOT(sl_onSequenceModified()));

    connect(loadFromFileToolButton, SIGNAL( clicked() ), SLOT( sl_onFileSelectorClicked()));
    connect(loadFromFileGroupBox, SIGNAL( toggled(bool) ), SLOT( sl_onFileSelectorToggled(bool)));

    connect(filePathLineEdit, SIGNAL(textChanged(const QString &)), SLOT(sl_activateNewSearch()));

    connect(getAnnotationsPushButton, SIGNAL(clicked()), SLOT(sl_getAnnotationsButtonClicked()));
    connect(prevPushButton, SIGNAL(clicked()), SLOT(sl_prevButtonClicked()));
    connect(nextPushButton, SIGNAL(clicked()), SLOT(sl_nextButtonClicked()));

    connect(useAmbiguousBasesBox, SIGNAL(toggled(bool)), SLOT(sl_activateNewSearch()));
    connect(spinMatch, SIGNAL(valueChanged(int)), SLOT(sl_activateNewSearch()));
}

void FindPatternWidget::sl_onAlgorithmChanged(int index)
{
    int previousAlgorithm = selectedAlgorithm;
    selectedAlgorithm = boxAlgorithm->itemData(index).toInt();
    updatePatternText(previousAlgorithm);
    updateLayout();
    verifyPatternAlphabet();
    sl_activateNewSearch(true);
}


void FindPatternWidget::sl_onRegionOptionChanged(int index)
{
    if (currentSelection != NULL){
        disconnect(currentSelection, SIGNAL(si_selectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&)),
            this, SLOT(sl_onSelectedRegionChanged()));
    }
    if (boxRegion->itemData(index).toInt() == RegionSelectionIndex_WholeSequence) {
        editStart->hide();
        lblStartEndConnection->hide();
        editEnd->hide();
        regionIsCorrect = true;
        checkState();
        setRegionToWholeSequence();
    }else if (boxRegion->itemData(index).toInt() == RegionSelectionIndex_CustomRegion) {
        editStart->show();
        lblStartEndConnection->show();
        editEnd->show();
        editStart->setReadOnly(false);
        editEnd->setReadOnly(false);

        ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
        SAFE_POINT(NULL != activeContext, "Internal error: there is no sequence in focus!",);
        getCompleteSearchRegion(regionIsCorrect, activeContext->getSequenceLength());
        checkState();
    }else if(boxRegion->itemData(index).toInt() == RegionSelectionIndex_CurrentSelectedRegion) {
        currentSelection = annotatedDnaView->getSequenceInFocus()->getSequenceSelection();
        connect(currentSelection, SIGNAL(si_selectionChanged(LRegionsSelection* , const QVector<U2Region>&, const QVector<U2Region>&)),
            this, SLOT(sl_onSelectedRegionChanged()) );
        editStart->show();
        lblStartEndConnection->show();
        editEnd->show();
        sl_onSelectedRegionChanged();
    }
}


void highlightBackground(QWidget* widget)
{
    widget->setStyleSheet(
        "background-color: " + L10N::errorColorTextFieldStr() + ";");
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
    } else {
        bool ok = false;
        qint64 value1 = editStart->text().toLongLong(&ok);
        if (!ok || (value1 < 1)) {
            highlightBackground(editStart);
            regionIsCorrect = false;
        }
        int value2 = editEnd->text().toLongLong(&ok);
        if (!ok || value2 < 1) {
            highlightBackground(editEnd);
            regionIsCorrect = false;
        }
    }

    if (regionIsCorrect) {
        doNotHighlightBackground(editStart);
        doNotHighlightBackground(editEnd);
    }

    boxRegion->setCurrentIndex(boxRegion->findData(RegionSelectionIndex_CustomRegion));

    checkState();
    if(regionIsCorrect){
        sl_activateNewSearch();
    }
}


void FindPatternWidget::sl_onFocusChanged(
    ADVSequenceWidget* /* prevWidget */, ADVSequenceWidget* currentWidget)
{
    Q_UNUSED(currentWidget);
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    if (activeContext != 0) {
        const DNAAlphabet* alphabet = activeContext->getAlphabet();
        isAminoSequenceSelected = alphabet->isAmino();
        updateLayout();

        // Update region
        setRegionToWholeSequence();
        doNotHighlightBackground(editStart);
        doNotHighlightBackground(editEnd);

        // Update available annotations table objects, etc.
        updateAnnotationsWidget();

        if(boxRegion->itemData(boxRegion->currentIndex()).toInt() == RegionSelectionIndex_CurrentSelectedRegion){
            disconnect(currentSelection, SIGNAL(si_selectionChanged(LRegionsSelection* , const QVector<U2Region>&, const QVector<U2Region>&)),
                this, SLOT(sl_onSelectedRegionChanged()) );
            currentSelection = annotatedDnaView->getSequenceInFocus()->getSequenceSelection();
            connect(currentSelection, SIGNAL(si_selectionChanged(LRegionsSelection* , const QVector<U2Region>&, const QVector<U2Region>&)),
                this, SLOT(sl_onSelectedRegionChanged()) );
             sl_onSelectedRegionChanged();
        }
    }
}


void FindPatternWidget::updateLayout()
{
    // Algorithm group
    if (selectedAlgorithm == FindAlgorithmPatternSettings_Exact) {
        useAmbiguousBasesBox->setChecked(false);
        useAmbiguousBasesContainer->hide();
        useMaxResultLenContainer->hide();
        boxMaxResultLen->hide();
        spinMatch->hide();
        lblMatch->hide();
    }
    if (selectedAlgorithm == FindAlgorithmPatternSettings_InsDel) {
        useAmbiguousBasesBox->setChecked(false);
        useAmbiguousBasesContainer->hide();
        useMaxResultLenContainer->hide();
        boxMaxResultLen->hide();
        enableDisableMatchSpin();
        lblMatch->show();
        spinMatch->show();
        QWidget::setTabOrder(boxAlgorithm, spinMatch);
        QWidget::setTabOrder(spinMatch, boxStrand);
    }
    else if (selectedAlgorithm == FindAlgorithmPatternSettings_Subst) {
        useAmbiguousBasesContainer->show();
        useMaxResultLenContainer->hide();
        boxMaxResultLen->hide();
        QWidget::setTabOrder(boxAlgorithm, spinMatch);
        QWidget::setTabOrder(spinMatch, useAmbiguousBasesBox);
        enableDisableMatchSpin();
        lblMatch->show();
        spinMatch->show();
        QWidget::setTabOrder(useAmbiguousBasesBox, boxStrand);
    }
    else if (selectedAlgorithm == FindAlgorithmPatternSettings_RegExp) {
        useAmbiguousBasesBox->setChecked(false);
        useAmbiguousBasesContainer->hide();
        useMaxResultLenContainer->show();
        boxMaxResultLen->show();
        spinMatch->hide();
        lblMatch->hide();
        QWidget::setTabOrder(boxAlgorithm, boxUseMaxResultLen);
        QWidget::setTabOrder(boxUseMaxResultLen, boxMaxResultLen);
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

void FindPatternWidget::showHideMessage( bool show, MessageFlag messageFlag, const QString& additionalMsg ){
    if (show) {
        if (!messageFlags.contains(messageFlag)) {
            messageFlags.append(messageFlag);
        }
    }
    else {
        messageFlags.removeAll(messageFlag);
    }

    if (!messageFlags.isEmpty()) {
        static QString storedTextColor = currentColorOfMessageText();
        if (storedTextColor != currentColorOfMessageText()) {
            changeColorOfMessageText(storedTextColor);
        }

#ifndef Q_OS_MAC
        const QString lineBreakShortcut = "Ctrl+Enter";
#else
        const QString lineBreakShortcut = "Cmd+Enter";
#endif
        QString text = "";
        foreach (MessageFlag flag, messageFlags) {
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
                case PatternsWithBadAlphabetInFile:
                    if (!text.isEmpty()) {
                        text += "\n";
                    }
                    text += QString(tr("Warning: file contains patterns that"
                        " do not match the active alphabet! Those patterns were ignored "));
                    break;
                case PatternsWithBadRegionInFile:
                    if (!text.isEmpty()) {
                        text += "\n";
                    }
                    text += QString(tr("Warning: file contains patterns that"
                        " longer than the search region! Those patterns were ignored. Please input a shorter value or select another region! "));
                    break;
                case UseMultiplePatternsTip:
                    if (!text.isEmpty()) {
                        text += "\n";
                    }
                    text += QString(tr("Info: please input at least one sequence pattern to search for. Use %1 to input multiple patterns.\r\n"
                        "Alternatively, load patterns from a FASTA file.").arg(lineBreakShortcut));
                    changeColorOfMessageText(L10N::infoHintColor().name());
                    break;
                case AnnotationNotValidName:
                    if (!text.isEmpty()) {
                        text += "\n";
                    }
                    text += QString(tr("Warning: annotation name or annotation group name are invalid. "));
                    if (!additionalMsg.isEmpty()){
                        text += QString(tr("Reason: "));
                        text += additionalMsg;
                    }
                    text += QString(tr(" Please input valid annotation names "));
                    break;
                case NoPatternToSearch:
                    if (!text.isEmpty()) {
                        text += "\n";
                    }
                    text += QString(tr("Warning: there is no pattern to search. "));
                    text += QString(tr(" Please input a valid pattern or choose a file with patterns "));
                    break;
                case SearchRegionIncorrect:
                    if (!text.isEmpty()) {
                        text += "\n";
                    }
                    text += QString(tr("Warning: there is no pattern to search. "));
                    text += QString(tr(" Please input a valid pattern or choose a file with patterns "));
                    break;
                case PatternWrongRegExp:
                    if (!text.isEmpty()) {
                        text += "\n";
                    }
                    text += QString(tr("Warning: invalid regexp. "));
                    break;
                default:
                    FAIL("Unexpected value of the error flag in show/hide error message for pattern!",);
            }
        }
        lblErrorMessage->setText(text);
    }
    else {
        lblErrorMessage->setText("");
    }
}

void FindPatternWidget::changeColorOfMessageText(const QString &newColorName)
{
    QString currentStyleSheet = lblErrorMessage->styleSheet();
    currentStyleSheet.replace(currentColorOfMessageText(), newColorName);
    lblErrorMessage->setStyleSheet(currentStyleSheet);
}

QString FindPatternWidget::currentColorOfMessageText() const
{
    const QString currentStyleSheet = lblErrorMessage->styleSheet();
    const int startOfColorDefinitionPosition = currentStyleSheet.indexOf(STYLESHEET_COLOR_DEFINITION);
    const int endOfColorDefinitionPosition = currentStyleSheet.indexOf(STYLESHEET_DEFINITIONS_SEPARATOR,
        startOfColorDefinitionPosition);
    const QString currentMessageTextColor = currentStyleSheet.mid(startOfColorDefinitionPosition
                                                                  + STYLESHEET_COLOR_DEFINITION.length(),
                                                                  endOfColorDefinitionPosition
                                                                  - startOfColorDefinitionPosition
                                                                  - STYLESHEET_COLOR_DEFINITION.length());
    return currentMessageTextColor;
}

void FindPatternWidget::sl_onSearchPatternChanged()
{
    static QString patterns = "";
    if (patterns != textPattern->toPlainText()) {
        patterns = textPattern->toPlainText();
        showHideMessage(patterns.isEmpty(), UseMultiplePatternsTip);

        setCorrectPatternsString();

        checkState();
        enableDisableMatchSpin();

        // Show a warning if the pattern alphabet doesn't match,
        // but do not block the "Search" button
        verifyPatternAlphabet();
        if(patterns != previousPatternString){
            previousPatternString = patterns;
            sl_activateNewSearch(false);
        }
    }
}

void FindPatternWidget::setCorrectPatternsString() {
    QTextCursor cursorInTextEdit = textPattern->textCursor();

    if (FindAlgorithmPatternSettings_RegExp != selectedAlgorithm) {
        FastaPatternsWalker walker(textPattern->toPlainText(), cursorInTextEdit.position());
        // Delete all non-alphabet symbols.
        while (walker.hasNext()) {
            QChar character(walker.next());
            if (walker.isCorrect()) {
                continue;
            }
            if (character.isLetter()) {
                if(!character.isUpper()) {
                    walker.setCurrent(character.toUpper().toLatin1());
                }
            } else {
                if ('\n' != character) {
                    walker.removeCurrent();
                }
            }
        }

        if (textPattern->toPlainText() != walker.getString()) {
            textPattern->setText(walker.getString());
            cursorInTextEdit.setPosition(walker.getCursor());
            textPattern->setTextCursor(cursorInTextEdit);
        }
    }
}

bool FindPatternWidget::hasWrongChars(const QString &input)
{
    FastaPatternsWalker walker(input);
    while (walker.hasNext()) {
        walker.next();
        if (!walker.isCorrect()) {
            return false;
        }
    }
    return true;
}

void FindPatternWidget::setRegionToWholeSequence()
{
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != activeContext, "Internal error: sequence context is NULL during setting region to whole sequence.",);

    editStart->setText(QString::number(1));
    editEnd->setText(QString::number(activeContext->getSequenceLength()));
    regionIsCorrect = true;
    boxRegion->setCurrentIndex(boxRegion->findData(RegionSelectionIndex_WholeSequence));
}


void FindPatternWidget::verifyPatternAlphabet()
{
    U2OpStatusImpl os;

    QStringList patternNoNames;
    QList<NamePattern > patternsWithNames = getPatternsFromTextPatternField(os);
    foreach (const NamePattern &name_pattern, patternsWithNames) {
        patternNoNames.append(name_pattern.second);
    }

    QString patterns = patternNoNames.join("");

    bool alphabetIsOk = checkAlphabet(patterns);

    if (!alphabetIsOk) {
        showHideMessage(true, PatternAlphabetDoNotMatch);
    }
    else {
        showHideMessage(false, PatternAlphabetDoNotMatch);
    }

    if(selectedAlgorithm == FindAlgorithmPatternSettings_RegExp){
        QRegExp regExp(textPattern->toPlainText());
        if(regExp.isValid()){
            showHideMessage(false, PatternWrongRegExp);
        }else{
            showHideMessage(true, PatternWrongRegExp);
        }
    }else{
        showHideMessage(false, PatternWrongRegExp);
    }
}

void FindPatternWidget::sl_onSequenceTranslationChanged(int /* index */)
{
    verifyPatternAlphabet();
}


void FindPatternWidget::sl_onSequenceModified()
{
    setRegionToWholeSequence();
    checkState();
    verifyPatternAlphabet();
}


void FindPatternWidget::checkState()
{
    //validate annotation name
    QString v = annotController->validate();
    if(!v.isEmpty()){
        showHideMessage(true, AnnotationNotValidName, v);
        annotController->setFocusToNameEdit();
        getAnnotationsPushButton->setDisabled(true);
        return;
    }
    if(usePatternNamesCheckBox->isChecked() && !loadFromFileGroupBox->isChecked()) {
        foreach(const QString &name, nameList){
            if (!Annotation::isValidAnnotationName(name)) {
                showHideMessage(true, AnnotationNotValidName);
                getAnnotationsPushButton->setDisabled(true);
                return;
            }
        }
        showHideMessage(false, AnnotationNotValidName);
    }

    getAnnotationsPushButton->setEnabled(!findPatternResults.isEmpty());

    // Disable the "Search" button if the pattern is empty
    //and pattern is not loaded from a file
    if (textPattern->toPlainText().isEmpty()
        && !loadFromFileGroupBox->isChecked()) {
        //showHideMessage(true, NoPatternToSearch);
        return;
    }

    // Show warning if the region is not correct
    if (!regionIsCorrect) {
        showHideMessage(true, SearchRegionIncorrect);
        return;
    }
    if(!loadFromFileGroupBox->isChecked()){
        // Show warning if the length of the pattern is greater than the search region length
        // Not for RegExp algorithm
        if (FindAlgorithmPatternSettings_RegExp != selectedAlgorithm) {
            bool regionOk = checkPatternRegion(textPattern->toPlainText());

            if (!regionOk) {
                highlightBackground(textPattern);
                showHideMessage(true, PatternIsTooLong);
                return;
            }
            else {
                doNotHighlightBackground(textPattern);
                showHideMessage(false, PatternIsTooLong);
            }
        }
    }

    showHideMessage(false, AnnotationNotValidName);
    showHideMessage(false, PatternsWithBadRegionInFile);
    showHideMessage(false, PatternsWithBadAlphabetInFile);
    showHideMessage(false, NoPatternToSearch);
    showHideMessage(false, SearchRegionIncorrect);
}


void FindPatternWidget::enableDisableMatchSpin()
{
    if ((textPattern->toPlainText().isEmpty() && !loadFromFileGroupBox->isChecked()) || isAminoSequenceSelected) {
        spinMatch->setEnabled(false);
    }
    else {
        spinMatch->setEnabled(true);
    }
}

U2Region FindPatternWidget::getCompleteSearchRegion(bool& regionIsCorrect, qint64 maxLen) const
{
    if (boxRegion->itemData(boxRegion->currentIndex()).toInt() == RegionSelectionIndex_WholeSequence) {
        regionIsCorrect = true;
        return U2Region(0, maxLen);
    }
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
        value2 += maxLen;
    }

    regionIsCorrect = true;
    return U2Region(value1, value2 - value1);
}

int FindPatternWidget::getMaxError( const QString& pattern ) const{
    if (selectedAlgorithm == FindAlgorithmPatternSettings_Exact) {
        return 0;
    }
    return int((float)(1 - float(spinMatch->value()) / 100) * pattern.length());
}

QList <QPair<QString, QString> > FindPatternWidget::getPatternsFromTextPatternField(U2OpStatus &os) const
{
    QString inputText = textPattern->toPlainText().toLocal8Bit();
    QList <NamePattern > result = FastaFormat::getSequencesAndNamesFromUserInput(inputText, os);

    if(result.isEmpty()) {
        QStringList patterns = inputText.split(QRegExp("\n"), QString::SkipEmptyParts);
        foreach(const QString& pattern, patterns){
            result.append(qMakePair(QString(""), pattern));
        }
    }

    if(!usePatternNamesCheckBox->isChecked()){
        const CreateAnnotationModel& model = annotController->getModel();
        for(int i = 0; i < result.size(); i++){
            result[i].first = model.data.name;
        }
    }

    return result;
}

void FindPatternWidget::updateAnnotationsWidget()
{
    // Updating the annotations widget
    SAFE_POINT(NULL != annotatedDnaView->getSequenceInFocus(),
        "There is no sequence in focus to update the annotations widget on the 'Search in Sequence' tab.",);
    CreateAnnotationModel newAnnotModel = annotController->getModel();

    if(!newAnnotModel.newDocUrl.isEmpty())
        newAnnotModel.newDocUrl = "";

    newAnnotModel.hideLocation = true;
    newAnnotModel.sequenceObjectRef = annotatedDnaView->getSequenceInFocus()->getSequenceObject();
    newAnnotModel.sequenceLen = annotatedDnaView->getSequenceInFocus()->getSequenceLength();
    annotController->updateWidgetForAnnotationModel(newAnnotModel);
}

#define FIND_PATTER_LAST_DIR "Find_pattern_last_dir"

void FindPatternWidget::sl_onFileSelectorClicked()
{
    LastUsedDirHelper lod(FIND_PATTER_LAST_DIR);

    QString filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true);
    lod.url = U2FileDialog::getOpenFileName(dynamic_cast<QWidget *>(AppContext::getMainWindow()),
                                           tr("Select file to open..."), lod.dir, filter);
    if (!lod.url.isEmpty())
        filePathLineEdit->setText(lod.url);
}

void FindPatternWidget::sl_onFileSelectorToggled(bool on)
{
    textPattern->setDisabled(on);
    checkState();
    sl_activateNewSearch(true);
}

void FindPatternWidget::initFindPatternTask(const QList<NamePattern> &patterns) {
    CHECK(!patterns.isEmpty(), );

    if (selectedAlgorithm == FindAlgorithmPatternSettings_RegExp) {
        QRegExp regExp(textPattern->toPlainText());
        CHECK(regExp.isValid(), );
    }
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != activeContext, "Internal error: there is no sequence in focus!",);

    FindAlgorithmTaskSettings settings;
    settings.sequence = activeContext->getSequenceObject()->getWholeSequenceData();
    settings.searchIsCircular = activeContext->getSequenceObject()->isCircular();

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
    settings.maxResult2Find =  boxMaxResult->value();

    // Region
    bool regionIsCorrectRef = false;
    U2Region region = getCompleteSearchRegion(regionIsCorrectRef, activeContext->getSequenceLength());
    SAFE_POINT(true == regionIsCorrectRef, "Internal error: incorrect search region has been supplied."
        " Skipping the pattern search.", );
    settings.searchRegion = region;

    // Algorithm settings
    settings.patternSettings = static_cast<FindAlgorithmPatternSettings>(selectedAlgorithm);

    settings.maxErr = 0;

    settings.useAmbiguousBases = useAmbiguousBasesBox->isChecked();
    settings.maxRegExpResult = boxUseMaxResultLen->isChecked() ?
        boxMaxResultLen->value() :
    DEFAULT_REGEXP_RESULT_LENGTH_LIMIT;

    // Creating and registering the task
    bool removeOverlaps = removeOverlapsBox->isChecked();

    SAFE_POINT(searchTask == NULL, "Search task is not NULL", );
    nextPushButton->setDisabled(true);
    prevPushButton->setDisabled(true);

    searchTask = new FindPatternListTask(settings,
        patterns,
        removeOverlaps,
        spinMatch->value());
    connect(searchTask, SIGNAL(si_stateChanged()), SLOT(sl_findPatrernTaskStateChanged()));

    AppContext::getTaskScheduler()->registerTopLevelTask(searchTask);
}

void FindPatternWidget::sl_loadPatternTaskStateChanged() {
    LoadPatternsFileTask *loadTask = qobject_cast<LoadPatternsFileTask *>(sender());
    CHECK(NULL != loadTask, );
    CHECK(loadTask->isFinished() && !loadTask->isCanceled(), );
    CHECK(!loadTask->hasError(), );

    QList<NamePattern> namesPatterns = loadTask->getNamesPatterns();
    nameList.clear();
    for (int i = 0; i < namesPatterns.size(); i++) {
        nameList << namesPatterns[i].first;
        namesPatterns[i].first = QString::number(i);
    }

    stopCurrentSearchTask();
    initFindPatternTask(namesPatterns);

    annotModelPrepared = false;
    updateAnnotationsWidget();
}

void FindPatternWidget::sl_findPatrernTaskStateChanged() {
    FindPatternListTask* findTask = qobject_cast<FindPatternListTask*>(sender());
    CHECK(NULL != findTask, );

    if (findTask->isFinished() || findTask->isCanceled() || findTask->hasError()) {
        findPatternResults = findTask->getResults();
        if (findPatternResults.isEmpty()) {
            resultLabel->setText(tr("Results: 0/0"));
        } else {
            iterPos = 1;
            qSort(findPatternResults.begin(), findPatternResults.end());
            resultLabel->setText(tr("Results: %1/%2").arg(QString::number(iterPos)).arg(QString::number(findPatternResults.size())));
            nextPushButton->setEnabled(true);
            prevPushButton->setEnabled(true);
            getAnnotationsPushButton->setEnabled(true);
            checkState();
            correctSearchInCombo();
            showCurrentResult();
        }
        if (findTask == searchTask) {
            disconnect(this, SLOT(sl_loadPatternTaskStateChanged()));
            searchTask = NULL;
        }
    }
}

bool FindPatternWidget::checkAlphabet( const QString& pattern ){
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != activeContext, "Internal error: there is no sequence in focus on pattern search!", false);

    const DNAAlphabet* alphabet = activeContext->getAlphabet();
    if (!isAminoSequenceSelected && SeqTranslIndex_Translation == boxSeqTransl->currentIndex()) {
        DNATranslation* translation = activeContext->getAminoTT();
        SAFE_POINT(NULL != translation, "Failed to get translation on pattern search!", false);

        alphabet = translation->getDstAlphabet();
    }

    bool alphabetIsOk = (TextUtils::fits(alphabet->getMap(),
        pattern.toLocal8Bit().data(),
        pattern.size()))    ||
        (useAmbiguousBasesBox->isChecked())   ||
        (FindAlgorithmPatternSettings_RegExp == selectedAlgorithm);

    return alphabetIsOk;
}

bool FindPatternWidget::checkPatternRegion( const QString& pattern ){
    int maxError = getMaxError(pattern);
    qint64 patternLength = pattern.length();
    qint64 minMatch = patternLength - maxError;
    SAFE_POINT(minMatch > 0, "Search pattern length is greater than max error value!",false);

    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != activeContext, "Internal error: there is no sequence in focus!", false);
    bool regionIsCorrect = false;
    qint64 regionLength = getCompleteSearchRegion(regionIsCorrect, activeContext->getSequenceLength()).length;

    SAFE_POINT(regionLength > 0 && true == regionIsCorrect,
               "Incorrect region length when enabling/disabling the pattern search button.", false);

    if (minMatch > regionLength) {
        return false;
    }
    return true;
}

void FindPatternWidget::sl_onSelectedRegionChanged(){
    if(!currentSelection->getSelectedRegions().isEmpty()){
        U2Region firstReg = currentSelection->getSelectedRegions().first();
        editStart->setText(QString::number(firstReg.startPos + 1));
        editEnd->setText(QString::number(firstReg.endPos()));

        if (currentSelection->getSelectedRegions().size() == 2) {
            U2Region secondReg = currentSelection->getSelectedRegions().last();
            SAFE_POINT(annotatedDnaView->getSequenceInFocus() != NULL, tr("Sequence in focus is NULL"), );
            int seqLen = annotatedDnaView->getSequenceInFocus()->getSequenceLength();
            bool circularSelection = (firstReg.startPos == 0 && secondReg.endPos() == seqLen)
                    || (firstReg.endPos() == seqLen && secondReg.startPos == 0);
            if (circularSelection) {
                if (secondReg.startPos == 0) {
                    editEnd->setText(QString::number(secondReg.endPos()));
                } else {
                    editStart->setText(QString::number(secondReg.startPos + 1));
                }
            }
        }
    }else{
        SAFE_POINT(annotatedDnaView->getSequenceInFocus() != NULL, "No sequence in focus, with active search tab in options panel",);
        editStart->setText(QString::number(1));
        editEnd->setText(QString::number(annotatedDnaView->getSequenceInFocus()->getSequenceLength()));
    }
    regionIsCorrect = true;
    boxRegion->setCurrentIndex(boxRegion->findData(RegionSelectionIndex_CustomRegion));
    checkState();
}

void FindPatternWidget::sl_onAnotationNameEdited(){
    checkState();
}

void FindPatternWidget::sl_onUsePatternNamesClicked(){
    bool isAnnotNamesEnabled = !usePatternNamesCheckBox->isChecked();
    annotController->setEnabledNameEdit(isAnnotNamesEnabled);
}

void FindPatternWidget::updatePatternText(int previousAlgorithm) {
    // Save a previous state.
    if (FindAlgorithmPatternSettings_RegExp == previousAlgorithm) {
        patternRegExp = textPattern->toPlainText();
    }
    else {
        patternString = textPattern->toPlainText();
    }

    // Set a new state.
    if (FindAlgorithmPatternSettings_RegExp == selectedAlgorithm) {
        textPattern->setText(patternRegExp);
    }
    else {
        textPattern->setText(patternString);
    }
    setCorrectPatternsString();
}

void FindPatternWidget::validateCheckBoxSize(QCheckBox* checkBox, int requiredWidth) {
    QFont font = checkBox->font();
    QFontMetrics checkBoxMetrics(font, checkBox);
    QString text = checkBox->text();

    if(text.contains('\n')) {
        return;
    }

    int lastSpacePos = 0;
    QString wrappedText = "";
    int startPos = 0;
    QRect textRect = checkBoxMetrics.boundingRect(text);
    if(textRect.width() <= requiredWidth) {
        return;
    }
    int length = text.length();
    for(int endPos = 0; endPos < length; endPos++) {
        if(' ' == text.at(endPos) || endPos == length - 1) {
            if(endPos-1 <= startPos) {
                wrappedText = "";
            }
            else {
                wrappedText = text.mid(startPos, endPos - startPos - 1);
            }
            textRect = checkBoxMetrics.boundingRect(wrappedText);
            if(textRect.width() > requiredWidth && 0 != lastSpacePos) {
                startPos = endPos;
                text[lastSpacePos] = '\n';
            }
            lastSpacePos = endPos;
        }
    }
    checkBox->setText(text);
}

void FindPatternWidget::sl_activateNewSearch(bool forcedSearch){
    if(loadFromFileGroupBox->isChecked()) {
        stopCurrentSearchTask();
        if(filePathLineEdit->text().isEmpty()){
            return;
        }
        LoadPatternsFileTask *loadTask = new LoadPatternsFileTask(filePathLineEdit->text());
        connect(loadTask, SIGNAL(si_stateChanged()), SLOT(sl_loadPatternTaskStateChanged()));
        AppContext::getTaskScheduler()->registerTopLevelTask(loadTask);
    } else {
        QList<NamePattern> newPatterns = updateNamePatterns();
        if(isSearchPatternsDifferent(newPatterns) || forcedSearch){
            patternList.clear();
            for(int i = 0; i < newPatterns.size();i++){
                newPatterns[i].first = QString::number(i);
                patternList.append(newPatterns[i].second);
            }
        }else{
            checkState();
            return;
        }

        stopCurrentSearchTask();
        initFindPatternTask(newPatterns);

        annotModelPrepared = false;
    }
}

QList<NamePattern> FindPatternWidget::updateNamePatterns(){
    CHECK(!loadFromFileGroupBox->isChecked(), QList<NamePattern>());

    U2OpStatus2Log os;
    QList<NamePattern> newPatterns = getPatternsFromTextPatternField(os);

    nameList.clear();
    foreach (const NamePattern &np, newPatterns) {
        nameList.append(np.first);
    }
    return newPatterns;
}

void FindPatternWidget::sl_getAnnotationsButtonClicked() {
    if (!annotModelPrepared){
        bool objectPrepared = annotController->prepareAnnotationObject();
        SAFE_POINT(objectPrepared, "Cannot create an annotation object. Please check settings", );
        annotModelPrepared = true;
    }
    QString v = annotController->validate();
    SAFE_POINT(v.isEmpty(), "Annotation names are invalid", );

    const CreateAnnotationModel& annotModel = annotController->getModel();
    QString group = annotModel.groupName;

    AnnotationTableObject *aTableObj = annotModel.getAnnotationObject();
    SAFE_POINT(aTableObj != NULL, "Invalid annotation table detected!", );

    QList<AnnotationData> annotationsToCreate = findPatternResults;

    for(int i = 0; i < findPatternResults.size(); i++){
        if(usePatternNamesCheckBox->isChecked()) {
            bool ok = false;
            int index = findPatternResults[i].name.toInt(&ok);
            if (Q_UNLIKELY(!ok)) {
                coreLog.details(tr("Warning: can not get valid pattern name, annotation will be named 'misc_feature'"));
                Q_ASSERT(false);
                if (annotationsToCreate[i].name.isEmpty()) {
                    annotationsToCreate[i].name = "misc_feature";
                }
            } else {
                SAFE_POINT(ok, "Failed conversion to integer", );
                SAFE_POINT(nameList.size() > index, "Out of boundaries in names list", );
                SAFE_POINT(index >= 0, "Out of boundaries in names list", );
                annotationsToCreate[i].name = nameList[index];
            }
        }else{
            annotationsToCreate[i].name = annotModel.data.name;
        }
    }
    GCOUNTER(cvar, tvar, "FindAlgorithmTask");
    AppContext::getTaskScheduler()->registerTopLevelTask(new CreateAnnotationsTask(aTableObj, group, annotationsToCreate));

    annotModelPrepared = false;
    updateAnnotationsWidget();
}

void FindPatternWidget::sl_prevButtonClicked() {
    int resultSize = findPatternResults.size();
    if(iterPos == 1){
        iterPos = resultSize;
    }else{
        iterPos--;
    }
    showCurrentResult();
}

void FindPatternWidget::sl_nextButtonClicked() {
    int resultSize = findPatternResults.size();
    if(iterPos == resultSize){
        iterPos = 1;
    }else{
        iterPos++;
    }
    showCurrentResult();
}

void FindPatternWidget::showCurrentResult() const {
    resultLabel->setText(tr("Results: %1/%2").arg(QString::number(iterPos)).arg(QString::number(findPatternResults.size())));
    CHECK(findPatternResults.size() >= iterPos, );
    const AnnotationData &ad = findPatternResults.at(iterPos-1);
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    const QVector<U2Region> regions = ad.getRegions();
    CHECK(activeContext->getSequenceSelection() != NULL, );
    CHECK(!regions.isEmpty(), );
    activeContext->getSequenceSelection()->setRegion(regions.first());
    int centerPos = regions.first().center() + 1;
    annotatedDnaView->sl_onPosChangeRequest(centerPos);
}

void FindPatternWidget::sl_onEnterPressed(){
    if(nextPushButton->isEnabled()){
        nextPushButton->click();
    }
}

void FindPatternWidget::sl_onShiftEnterPressed(){
    if(prevPushButton->isEnabled()){
        prevPushButton->click();
    }
}

void FindPatternWidget::sl_usePatternNamesCbClicked(){
    updateNamePatterns();
    checkState();
}

bool FindPatternWidget::isSearchPatternsDifferent(const QList<NamePattern> &newPatterns) const {
    if(patternList.size() != newPatterns.size()){
        return true;
    }
    foreach (const NamePattern &s, newPatterns) {
        if (!patternList.contains(s.second)) {
            return true;
        }
    }
    return false;
}

void FindPatternWidget::stopCurrentSearchTask(){
    if(searchTask != NULL){
        disconnect(this, SLOT(sl_loadPatternTaskStateChanged()));
        if(!searchTask->isCanceled() && searchTask->getState() != Task::State_Finished){
            searchTask->cancel();
        }
        searchTask = NULL;
    }
    findPatternResults.clear();
    nextPushButton->setDisabled(true);
    prevPushButton->setDisabled(true);
    getAnnotationsPushButton->setDisabled(true);
}

void FindPatternWidget::correctSearchInCombo(){
    if(boxRegion->itemData(boxRegion->currentIndex()).toInt() == RegionSelectionIndex_CurrentSelectedRegion){
        boxRegion->setCurrentIndex(boxRegion->findData(RegionSelectionIndex_CustomRegion));
    }
}

void FindPatternWidget::setUpTabOrder() const {
    CreateAnnotationWidgetController *annotWidget = qobject_cast<CreateAnnotationWidgetController*>(annotController);
    SAFE_POINT(annotWidget != NULL, "Bad casting to CreateAnnotationWidgetController", );

    QWidget::setTabOrder(nextPushButton, boxAlgorithm);
    QWidget::setTabOrder(boxAlgorithm, boxStrand);
    QWidget::setTabOrder(boxStrand, boxSeqTransl);
    QWidget::setTabOrder(boxSeqTransl, boxRegion);
    QWidget::setTabOrder(boxRegion, editStart);
    QWidget::setTabOrder(editStart, editEnd);
    QWidget::setTabOrder(editEnd, removeOverlapsBox);
    QWidget::setTabOrder(removeOverlapsBox, boxMaxResult);
    QWidget::setTabOrder(boxMaxResult, annotWidget->getTaborderEntryAndExitPoints().first);
    QWidget::setTabOrder(annotWidget->getTaborderEntryAndExitPoints().second, usePatternNamesCheckBox);
    QWidget::setTabOrder(usePatternNamesCheckBox, getAnnotationsPushButton);
}

int FindPatternWidget::getTargetSequnceLength() const {
    SAFE_POINT(annotatedDnaView->getSequenceInFocus() != NULL, "Sequence is NULL", 0);
    return annotatedDnaView->getSequenceInFocus()->getSequenceLength();
}

} // namespace
