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

#include "FindPatternTask.h"
#include "FindPatternWidget.h"

#include <U2Algorithm/FindAlgorithmTask.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AnnotationData.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceWidget.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/ShowHideSubgroupWidget.h>

#include <U2Core/DocumentUtils.h>
#include <U2Gui/DialogUtils.h>
#include <U2Formats/FastaFormat.h>
#include <U2Formats/GenbankFeatures.h>


const QString NEW_LINE_SYMBOL = "\n";
const QString COLOR_NAME_FOR_INFO_MESSAGES = "green";
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

class RegExpPatternsWalker {
public:
    RegExpPatternsWalker(const QString &_patternsString, int _cursor = 0)
        : patternsString(_patternsString.toLatin1()),
          cursor(_cursor),
          current(-1),
          comment(false),
          header(false)
    {
        // RegExp alphabet:
        regExpSlashedSymbols = "abBfnrtvx0dDsSwW";
        regExpUnslashedSymbols = "^$.,+*{}[]()?!-|&\\:=";
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
        return patternsString[current];
    }

    bool isSequenceChar() const {
        CHECK(-1 != current, false);
        return !isRegExpChar() && !isMetaChars();
    }

    bool isRegExpChar() const {
        CHECK(-1 != current, false);
        if (0 == current) {
            if (regExpUnslashedSymbols.contains(patternsString[current])) {
                return true;
            }
        }
        else {
            // current > 0
            if (regExpUnslashedSymbols.contains(patternsString[current])) {
                return true;
            }
            if (regExpSlashedSymbols.contains(patternsString[current]) &&
                    '\\' == patternsString[current - 1]) {
                return true;
            }
            if (QChar(patternsString[current]).isDigit() &&
                    QChar(patternsString[current - 1]).isDigit()) {
                return true;
            }
            // It is backreference:
            if (QChar(patternsString[current]).isDigit() &&
                '\\' == patternsString[current - 1]) {
                return true;
            }
            // \0ooo (i.e., \zero ooo)
            if (QChar(patternsString[current]).isDigit() &&
                    '0' == patternsString[current - 1]) {
                return true;
            }
            // \xhhhh
            if (current > 1) {
                if (QChar(patternsString[current]).isDigit() &&
                        'x' == patternsString[current - 1] &&
                        '\\' == patternsString[current - 2]) {
                    return true;
                }
            }

        }
        return false;
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
    QString regExpSlashedSymbols;
    QString regExpUnslashedSymbols;

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
        if (Qt::Key_Tab == keyEvent->key()) {
            emit si_tabPressed();
            return true;
        }
        if ((Qt::Key_Enter == keyEvent->key() || Qt::Key_Return == keyEvent->key())) {
            if(!(Qt::ControlModifier & keyEvent->modifiers())) {
                emit si_enterPressed();
                return true;
            }
            else {
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

const QString FindPatternWidget::ALGORITHM_SETTINGS = tr("Search algorithm");
const QString FindPatternWidget::SEARCH_IN_SETTINGS = tr("Search in");
const QString FindPatternWidget::OTHER_SETTINGS= tr("Other settings");

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

        annotController = new CreateAnnotationWidgetController(annotModel, this, optPanel);
        annotModelPrepared = false;
        connect(annotController, SIGNAL(si_annotationNamesEdited()), SLOT(sl_onAnotationNameEdited()));

        setContentsMargins(0, 0, 0, 0);
        
        annotsWidget = annotController->getWidget();
        usePatternNamesCheckBox = annotController->getUsePatternNameCheckBox();
        annotController->setUsePatternNameCheckBoxVisible();

        const DNAAlphabet* alphabet = activeContext->getAlphabet();
        isAminoSequenceSelected = alphabet->isAmino();

        initLayout();
        connectSlots();

        checkState();
        btnSearch->setAutoDefault(true);

        FindPatternEventFilter *findPatternEventFilter = new FindPatternEventFilter(this);
        textPattern->installEventFilter(findPatternEventFilter);
        connect(findPatternEventFilter, SIGNAL(si_tabPressed()), SLOT(sl_onTabInPatternFieldPressed()));
        connect(findPatternEventFilter, SIGNAL(si_enterPressed()), SLOT(sl_onEnterInPatternFieldPressed()));
        
        setFocusProxy(textPattern);
        
        currentSelection = NULL;
        showHideMessage(true, UseMultiplePatternsTip);

        sl_onSearchPatternChanged();
    }
}

void FindPatternWidget::initLayout()
{
    setMinimumSize(QSize(170, 150));

    initAlgorithmLayout();
    initStrandSelection();
    initSeqTranslSelection();
    initRegionSelection();
    initResultsLimit();

    subgroupsLayout->setSpacing(0);
    subgroupsLayout->addWidget(new ShowHideSubgroupWidget(ALGORITHM_SETTINGS, ALGORITHM_SETTINGS, widgetAlgorithm, true));
    subgroupsLayout->addWidget(new ShowHideSubgroupWidget(SEARCH_IN_SETTINGS, SEARCH_IN_SETTINGS, widgetSearchIn, true));
    subgroupsLayout->addWidget(new ShowHideSubgroupWidget(OTHER_SETTINGS, OTHER_SETTINGS, widgetOther, false));
    subgroupsLayout->addWidget(annotsWidget);

    lblErrorMessage->setStyleSheet(
        "color: " + L10N::errorColorLabelStr() + ";"
        "font: bold;");
    lblErrorMessage->setText("");

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

    useAmbiguousBasesBox = new QCheckBox(tr("Search with ambiguous bases"));
    layoutAlgorithmSettings->addWidget(useAmbiguousBasesBox);

    layoutRegExpLen = new QHBoxLayout();
    boxUseMaxResultLen = new QCheckBox(tr("Results no longer than"));
    boxMaxResultLen = new QSpinBox();
    boxMaxResultLen->setMinimum(REG_EXP_MIN_RESULT_LEN);
    boxMaxResultLen->setMaximum(REG_EXP_MAX_RESULT_LEN);
    boxMaxResultLen->setSingleStep(REG_EXP_MAX_RESULT_SINGLE_STEP);
    boxMaxResultLen->setValue(REG_EXP_MAX_RESULT_LEN);
    layoutRegExpLen->addWidget(boxUseMaxResultLen);
    layoutRegExpLen->addWidget(boxMaxResultLen);
    layoutAlgorithmSettings->addLayout(layoutRegExpLen);
    boxMaxResultLen->setEnabled( false );
    connect( boxUseMaxResultLen, SIGNAL( toggled( bool ) ), boxMaxResultLen,
        SLOT( setEnabled( bool ) ) );

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
    boxRegion->addItem(tr("Whole sequence"), RegionSelectionIndex_WholeSequence);
    boxRegion->addItem(tr("Custom region"), RegionSelectionIndex_CustomRegion);
    boxRegion->addItem(tr("Selected region"), RegionSelectionIndex_CurrentSelectedRegion);

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
    boxMaxResult->setMinimum(0);
    boxMaxResult->setMaximum(INT_MAX);
    boxMaxResult->setValue(DEFAULT_RESULTS_NUM_LIMIT);
    boxMaxResult->setEnabled(false);
}


void FindPatternWidget::connectSlots()
{
    connect(boxAlgorithm, SIGNAL(currentIndexChanged(int)), SLOT(sl_onAlgorithmChanged(int)));
    connect(boxRegion, SIGNAL(currentIndexChanged(int)), SLOT(sl_onRegionOptionChanged(int)));
    connect(textPattern, SIGNAL(textChanged()), SLOT(sl_onSearchPatternChanged()));
    connect(boxUseMaxResult, SIGNAL(stateChanged(int)), SLOT(sl_enableBoxMaxResult(int)));
    connect(editStart, SIGNAL(textEdited(QString)), SLOT(sl_onRegionValueEdited()));
    connect(editEnd, SIGNAL(textEdited(QString)), SLOT(sl_onRegionValueEdited()));
    connect(boxSeqTransl, SIGNAL(currentIndexChanged(int)), SLOT(sl_onSequenceTranslationChanged(int)));
    connect(btnSearch, SIGNAL(clicked()), SLOT(sl_onSearchClicked()));

    // A sequence has been selected in the Sequence View
    connect(annotatedDnaView, SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)),
        this, SLOT(sl_onFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*)));

    // A sequence has been modified (a subsequence added, removed, etc.)
    connect(annotatedDnaView, SIGNAL(si_sequenceModified(ADVSequenceObjectContext*)),
        this, SLOT(sl_onSequenceModified(ADVSequenceObjectContext*)));

    connect(loadFromFileToolButton, SIGNAL( clicked() ), SLOT( sl_onFileSelectorClicked()));
    connect(loadFromFileGroupBox, SIGNAL( toggled(bool) ), SLOT( sl_onFileSelectorToggled(bool)));

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

void FindPatternWidget::sl_onAlgorithmChanged(int index)
{
    int previousAlgorithm = selectedAlgorithm;
    selectedAlgorithm = boxAlgorithm->itemData(index).toInt();
    updatePatternText(previousAlgorithm);
    updateLayout();
    verifyPatternAlphabet();
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
    }
    else if (boxRegion->itemData(index).toInt() == RegionSelectionIndex_CustomRegion) {
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

    boxRegion->setCurrentIndex(boxRegion->findData(RegionSelectionIndex_CustomRegion));

    checkState();
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
        useAmbiguousBasesBox->hide();
        boxUseMaxResultLen->hide();
        boxMaxResultLen->hide();
        spinMatch->hide();
        lblMatch->hide();
    }
    if (selectedAlgorithm == FindAlgorithmPatternSettings_InsDel) {
        useAmbiguousBasesBox->setChecked(false);
        useAmbiguousBasesBox->hide();
        boxUseMaxResultLen->hide();
        boxMaxResultLen->hide();
        enableDisableMatchSpin();
        lblMatch->show();
        spinMatch->show();
        QWidget::setTabOrder(boxAlgorithm, spinMatch);
    }
    else if (selectedAlgorithm == FindAlgorithmPatternSettings_Subst) {
        useAmbiguousBasesBox->show();
        boxUseMaxResultLen->hide();
        boxMaxResultLen->hide();
        QWidget::setTabOrder(boxAlgorithm, useAmbiguousBasesBox);
        enableDisableMatchSpin();
        lblMatch->show();
        spinMatch->show();
        QWidget::setTabOrder(spinMatch, useAmbiguousBasesBox);
    }
    else if (selectedAlgorithm == FindAlgorithmPatternSettings_RegExp) {
        useAmbiguousBasesBox->setChecked(false);
        useAmbiguousBasesBox->hide();
        boxUseMaxResultLen->show();
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
        if(storedTextColor != currentColorOfMessageText())
            changeColorOfMessageText(storedTextColor);

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
                    text += QString(tr("Info: You may use Ctrl+Enter"
                        " to input multiple patterns "));
                    changeColorOfMessageText(COLOR_NAME_FOR_INFO_MESSAGES);
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
    } else {
        // Allow the alphabet and regexp symbols only.
        RegExpPatternsWalker walker(textPattern->toPlainText(), cursorInTextEdit.position());
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
                walker.removeCurrent();
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
}


void FindPatternWidget::verifyPatternAlphabet()
{
    U2OpStatusImpl os;
    
    QStringList patternNoNames;
    QList<NamePattern > patternsWithNames = getPatternsFromTextPatternField(os);
    foreach(const NamePattern& name_pattern, patternsWithNames ){
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
    //and pattern is not loaded from a file
    if (textPattern->toPlainText().isEmpty()
        && !loadFromFileGroupBox->isChecked()) {
        btnSearch->setDisabled(true);
        return;
    }

    // Disable if the region is not correct
    if (!regionIsCorrect) {
        btnSearch->setDisabled(true);
        return;
    }
    if(!loadFromFileGroupBox->isChecked()){
        // Disable if the length of the pattern is greater than the search region length
        // Not for RegExp algorithm
        if (FindAlgorithmPatternSettings_RegExp != selectedAlgorithm) {
            bool regionOk = checkPatternRegion(textPattern->toPlainText());

            if (!regionOk) {
                btnSearch->setDisabled(true);
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

    //validate annotation name
    QString v = annotController->validate();
    if(!v.isEmpty()){
        btnSearch->setDisabled(true);
        showHideMessage(true, AnnotationNotValidName, v);
        annotController->setFocusToNameEdit();
        return;
    }

    showHideMessage(false, AnnotationNotValidName);
    showHideMessage(false, PatternsWithBadRegionInFile);
    showHideMessage(false, PatternsWithBadAlphabetInFile);
    // Otherwise enable the button
    btnSearch->setDisabled(false);
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
        regionIsCorrect = false;
        return U2Region();
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

void FindPatternWidget::sl_onTabInPatternFieldPressed()
{
    if (btnSearch->isEnabled()) {
        btnSearch->setFocus(Qt::TabFocusReason);
    } else {
        boxAlgorithm->setFocus(Qt::TabFocusReason);
    }
    return;
}

void FindPatternWidget::sl_onEnterInPatternFieldPressed()
{
    if (btnSearch->isEnabled()) {
        sl_onSearchClicked();
    }
    else {
        return;
    }
}

void FindPatternWidget::sl_onSearchClicked()
{
    SAFE_POINT(!textPattern->toPlainText().isEmpty() || loadFromFileGroupBox->isChecked(), "Internal error: can't search for an empty string!",);
    
    if(loadFromFileGroupBox->isChecked()) {
        LoadPatternsFileTask* loadTask = new LoadPatternsFileTask(filePathLineEdit->text());
        connect(loadTask, SIGNAL(si_stateChanged()), SLOT(sl_loadPatternTaskStateChanged()));
        AppContext::getTaskScheduler()->registerTopLevelTask(loadTask);
    } else {
        U2OpStatus2Log os;
        const QList <NamePattern >& patterns = getPatternsFromTextPatternField(os);

        initFindPatternTask(patterns);
        updateAnnotationsWidget();

        annotModelPrepared = false;
    }
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
    lod.url = QFileDialog::getOpenFileName(dynamic_cast<QWidget *>(AppContext::getMainWindow()),
                                           tr("Select file to open..."), lod.dir, filter);
    if (!lod.url.isEmpty())
        filePathLineEdit->setText(lod.url);
}

void FindPatternWidget::sl_onFileSelectorToggled(bool on)
{
    textPattern->setDisabled(on);
    checkState();
}

void FindPatternWidget::initFindPatternTask( const QList<NamePattern>& patterns){
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != activeContext, "Internal error: there is no sequence in focus!",);

    FindAlgorithmTaskSettings settings;
    settings.sequence = activeContext->getSequenceObject()->getWholeSequenceData();

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

    // Algorithm settings
    settings.patternSettings = static_cast<FindAlgorithmPatternSettings>(selectedAlgorithm);

    settings.maxErr = 0;

    settings.useAmbiguousBases = useAmbiguousBasesBox->isChecked();
    settings.maxRegExpResult = boxUseMaxResultLen->isChecked() ?
        boxMaxResultLen->value() :
    DEFAULT_REGEXP_RESULT_LENGTH_LIMIT;

    // Preparing the annotations object and other annotations parameters
    if (!annotModelPrepared){

        bool objectPrepared = annotController->prepareAnnotationObject();
        SAFE_POINT(objectPrepared, "Cannot create an annotation object. Please check settings", );
        annotModelPrepared = true;
    }

    QString v = annotController->validate();
    SAFE_POINT(v.isEmpty(), "Annotation names are invalid", );

    const CreateAnnotationModel& annotModel = annotController->getModel();
    QString annotGroup = annotModel.groupName;

    AnnotationTableObject *aTableObj = annotModel.getAnnotationObject();
    SAFE_POINT(aTableObj != NULL, "Invalid annotation table detected!", );

    // Creating and registering the task
    bool removeOverlaps = removeOverlapsBox->isChecked();

    FindPatternListTask* task = new FindPatternListTask(settings,
        patterns,
        aTableObj,
        annotModel.data.name,
        annotGroup,
        removeOverlaps,
        spinMatch->value(),
        usePatternNamesCheckBox->isChecked());
    connect(task, SIGNAL(si_progressChanged()), SLOT(sl_findPatrernTaskStateChanged()));

    AppContext::getTaskScheduler()->registerTopLevelTask(task);
}

void FindPatternWidget::sl_loadPatternTaskStateChanged(){
    LoadPatternsFileTask* loadTask = qobject_cast<LoadPatternsFileTask*>(sender());
    if (!loadTask){
        return;
    }
    if(!loadTask->isFinished() || loadTask->isCanceled()){
        return;
    }
    if (loadTask->hasError()){

        return;
    }
    bool noBadRegion = true;
    bool noBadAlphabet = true;

    const QList<NamePattern>& namesPatterns = loadTask->getNamesPatterns();
    initFindPatternTask(namesPatterns);

    if(!noBadAlphabet){
        showHideMessage(true, PatternsWithBadAlphabetInFile);
    }
    if(!noBadRegion){
        showHideMessage(true, PatternsWithBadRegionInFile);
    }
    annotModelPrepared = false;
    updateAnnotationsWidget();
}

void FindPatternWidget::sl_findPatrernTaskStateChanged() {
    FindPatternListTask* findTask = qobject_cast<FindPatternListTask*>(sender());
    if (!findTask) {
        return;
    }
    if(!findTask->isFinished() || findTask->isCanceled()){
        return;
    }
    if (findTask->hasError()){
        return;
    }
    updateAnnotationsWidget();
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
        U2Region r = currentSelection->getSelectedRegions().first();
        editStart->setText(QString::number(r.startPos + 1));
        editEnd->setText(QString::number(r.endPos() + 1));
    }else{
        SAFE_POINT(annotatedDnaView->getSequenceInFocus() != NULL, "No sequence in focus, with active search tab in options panel",);
        editStart->setText(QString::number(1));
        editEnd->setText(QString::number(annotatedDnaView->getSequenceInFocus()->getSequenceLength()));
    }
    regionIsCorrect = true;
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
    QFontMetrics checkBoxMetrics(font); 
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

} // namespace
