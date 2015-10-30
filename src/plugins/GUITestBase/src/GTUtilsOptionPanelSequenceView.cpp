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

#include <QDir>
#include <QLabel>
#include <QTextEdit>
#include <QTreeWidget>

#include <U2Core/global.h>
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsOptionPanelSequenceView.h"
#include "GTUtilsTaskTreeView.h"
#include <primitives/GTCheckBox.h>
#include "system/GTClipboard.h"
#include <primitives/GTComboBox.h>
#include <drivers/GTKeyboardDriver.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTSpinBox.h>
#include <primitives/GTTableView.h>
#include <primitives/GTTextEdit.h>
#include "utils/GTThread.h"
#include <primitives/GTWidget.h>

namespace U2 {
using namespace HI;

QMap<GTUtilsOptionPanelSequenceView::Tabs, QString> GTUtilsOptionPanelSequenceView::initNames() {
    QMap<Tabs,QString> result;
    result.insert(Search, "OP_FIND_PATTERN");
    result.insert(AnnotationsHighlighting, "OP_ANNOT_HIGHLIGHT");
    result.insert(Statistics, "OP_SEQ_INFO");
    result.insert(InSilicoPcr, "OP_IN_SILICO_PCR");
    result.insert(CircularView, "OP_CV_SETTINGS");
    result.insert(Das, "OP_DAS");
    return result;
}

QMap<GTUtilsOptionPanelSequenceView::Tabs, QString> GTUtilsOptionPanelSequenceView::initInnerWidgetNames() {
    QMap<Tabs, QString> result;
    result.insert(Search, "FindPatternWidget");
    result.insert(AnnotationsHighlighting, "AnnotHighlightWidget");
    result.insert(Statistics, "SequenceInfo");
    result.insert(InSilicoPcr, "InSilicoPcrOptionPanelWidget");
    result.insert(CircularView, "CircularViewSettingsWidget");
    result.insert(Das, "DasOptionsPanelWidget");
    return result;
}

const QMap<GTUtilsOptionPanelSequenceView::Tabs,QString> GTUtilsOptionPanelSequenceView::tabsNames = initNames();
const QMap<GTUtilsOptionPanelSequenceView::Tabs, QString> GTUtilsOptionPanelSequenceView::innerWidgetNames = initInnerWidgetNames();

#define GT_CLASS_NAME "GTUtilsOptionPanelSequenceView"

#define GT_METHOD_NAME "enterPattern"

void GTUtilsOptionPanelSequenceView::enterPattern( U2OpStatus &os, QString pattern, bool useCopyPaste ){
    QTextEdit *patternEdit = qobject_cast<QTextEdit*>(GTWidget::findWidget(os, "textPattern"));
    GTWidget::click(os, patternEdit);

    GTTextEdit::clear(os, patternEdit);
    if(useCopyPaste){
        GTClipboard::setText(os, pattern);
        GTKeyboardDriver::keyClick(os, 'v', GTKeyboardDriver::key["ctrl"]);
    }else{
        GTTextEdit::setText(os, patternEdit, pattern);
    }

    GTGlobals::sleep(3000);
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleTab"
void GTUtilsOptionPanelSequenceView::toggleTab(U2OpStatus &os, GTUtilsOptionPanelSequenceView::Tabs tab) {
    GTWidget::click(os, GTWidget::findWidget(os, tabsNames[tab], GTUtilsMdi::activeWindow(os)));
    GTGlobals::sleep(200);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openTab"
void GTUtilsOptionPanelSequenceView::openTab(U2OpStatus &os, Tabs tab){
    if (!isTabOpened(os, tab)) {
        toggleTab(os, tab);
        GTThread::waitForMainThread(os);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "closeTab"
void GTUtilsOptionPanelSequenceView::closeTab(U2OpStatus &os, GTUtilsOptionPanelSequenceView::Tabs tab) {
    if (isTabOpened(os, tab)) {
        toggleTab(os, tab);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isTabOpened"
bool GTUtilsOptionPanelSequenceView::isTabOpened(U2OpStatus &os, GTUtilsOptionPanelSequenceView::Tabs tab) {
    GTGlobals::FindOptions options;
    options.failIfNull = false;
    QWidget *innerTabWidget = GTWidget::findWidget(os, innerWidgetNames[tab], GTUtilsMdi::activeWindow(os), options);
    return NULL != innerTabWidget && innerTabWidget->isVisible();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkResultsText"

bool GTUtilsOptionPanelSequenceView::checkResultsText(U2OpStatus &os, QString expectedText) {
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QLabel *label = qobject_cast<QLabel*>(GTWidget::findWidget(os, "resultLabel"));
    return label->text() == expectedText;
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setSearchWithAmbiguousBases"
void GTUtilsOptionPanelSequenceView::setSearchWithAmbiguousBases(U2OpStatus &os, bool searchWithAmbiguousBases) {
    GTCheckBox::setChecked(os, GTWidget::findExactWidget<QCheckBox *>(os, "useAmbiguousBasesBox"), searchWithAmbiguousBases);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickNext"

void GTUtilsOptionPanelSequenceView::clickNext(U2OpStatus &os){
    QPushButton *next = qobject_cast<QPushButton*>(GTWidget::findWidget(os, "nextPushButton"));
    GTWidget::click(os, next);
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickPrev"

void GTUtilsOptionPanelSequenceView::clickPrev(U2OpStatus &os){
    QPushButton *prev = qobject_cast<QPushButton*>(GTWidget::findWidget(os, "prevPushButton"));
    GTWidget::click(os, prev);
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickGetAnnotation"

void GTUtilsOptionPanelSequenceView::clickGetAnnotation(U2OpStatus &os){
    QPushButton *getAnnotations = qobject_cast<QPushButton*>(GTWidget::findWidget(os, "getAnnotationsPushButton"));
    GTWidget::click(os, getAnnotations);
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isPrevNextEnabled"

bool GTUtilsOptionPanelSequenceView::isPrevNextEnabled(U2OpStatus &os){
    QPushButton *next = qobject_cast<QPushButton*>(GTWidget::findWidget(os, "nextPushButton"));
    QPushButton *prev = qobject_cast<QPushButton*>(GTWidget::findWidget(os, "prevPushButton"));
    return prev->isEnabled() && next->isEnabled();
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isGetAnnotationsEnabled"
bool GTUtilsOptionPanelSequenceView::isGetAnnotationsEnabled(U2OpStatus &os){
    QPushButton *getAnnotations = qobject_cast<QPushButton*>(GTWidget::findWidget(os, "getAnnotationsPushButton"));
    return getAnnotations->isEnabled();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleCircularView"
void GTUtilsOptionPanelSequenceView::toggleCircularView(U2OpStatus &os) {
    GTWidget::click(os, GTWidget::findButtonByText(os, "Open Circular View(s)", GTUtilsMdi::activeWindow(os)));
    GTThread::waitForMainThread(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setTitleFontSize"
void GTUtilsOptionPanelSequenceView::setTitleFontSize(U2OpStatus &os, int fontSize) {
    GTSpinBox::setValue(os, GTWidget::findExactWidget<QSpinBox *>(os, "fontSizeSpinBox"), fontSize);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getTitleFontSize"
int GTUtilsOptionPanelSequenceView::getTitleFontSize(U2OpStatus &os) {
    return GTSpinBox::getValue(os, GTWidget::findExactWidget<QSpinBox *>(os, "fontSizeSpinBox", GTUtilsMdi::activeWindow(os)));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "fetchDasIds"
void GTUtilsOptionPanelSequenceView::fetchDasIds(U2OpStatus &os) {
    openTab(os, Das);
    GTWidget::click(os, GTWidget::findWidget(os, "searchIdsButton"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getDasIdsCount"
int GTUtilsOptionPanelSequenceView::getDasIdsCount(U2OpStatus &os) {
    openTab(os, Das);
    return GTTableView::rowCount(os, GTWidget::findExactWidget<QTableView *>(os, "idList"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setForwardPrimer"
void GTUtilsOptionPanelSequenceView::setForwardPrimer(U2OpStatus &os, const QString &primer) {
    openTab(os, InSilicoPcr);
    QWidget *primerContainer = GTWidget::findWidget(os, "forwardPrimerBox");
    GT_CHECK(NULL != primerContainer, "Forward primer container widget is NULL");
    GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "primerEdit", primerContainer), primer);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setReversePrimer"
void GTUtilsOptionPanelSequenceView::setReversePrimer(U2OpStatus &os, const QString &primer) {
    openTab(os, InSilicoPcr);
    QWidget *primerContainer = GTWidget::findWidget(os, "reversePrimerBox");
    GT_CHECK(NULL != primerContainer, "Reverse primer container widget is NULL");
    GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "primerEdit", primerContainer), primer);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "showPrimersDetails"
void GTUtilsOptionPanelSequenceView::showPrimersDetails(U2OpStatus &os) {
    openTab(os, InSilicoPcr);
    GTWidget::clickLabelLink(os, GTWidget::findWidget(os, "detailsLinkLabel"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isSearchAlgorithmShowHideWidgetOpened"
bool GTUtilsOptionPanelSequenceView::isSearchAlgorithmShowHideWidgetOpened(U2OpStatus &os) {
    QWidget *algorithmInnerWidget = GTWidget::findWidget(os, "widgetAlgorithm");
    GT_CHECK_RESULT(NULL != algorithmInnerWidget, "algorithmInnerWidget is NULL", false);
    return algorithmInnerWidget->isVisible();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isSearchInShowHideWidgetOpened"
bool GTUtilsOptionPanelSequenceView::isSearchInShowHideWidgetOpened(U2OpStatus &os) {
    QWidget *searchInInnerWidget = GTWidget::findWidget(os, "widgetSearchIn");
    GT_CHECK_RESULT(NULL != searchInInnerWidget, "searchInInnerWidget is NULL", false);
    return searchInInnerWidget->isVisible();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isOtherSettingsShowHideWidgetOpened"
bool GTUtilsOptionPanelSequenceView::isOtherSettingsShowHideWidgetOpened(U2OpStatus &os) {
    QWidget *otherSettingsInnerWidget = GTWidget::findWidget(os, "widgetOther");
    GT_CHECK_RESULT(NULL != otherSettingsInnerWidget, "otherSettingsInnerWidget is NULL", false);
    return otherSettingsInnerWidget->isVisible();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isSaveAnnotationToShowHideWidgetOpened"
bool GTUtilsOptionPanelSequenceView::isSaveAnnotationToShowHideWidgetOpened(U2OpStatus &os) {
    QWidget *annotationsWidget = GTWidget::findWidget(os, "annotationsWidget");
    GT_CHECK_RESULT(NULL != annotationsWidget, "annotationsWidget is NULL", false);
    QWidget *saveAnnotationsToInnerWidget = GTWidget::findWidget(os, "saveAnnotationsInnerWidget", annotationsWidget);
    GT_CHECK_RESULT(NULL != saveAnnotationsToInnerWidget, "saveAnnotationsInnerWidget is NULL", false);
    return saveAnnotationsToInnerWidget->isVisible();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isAnnotationParametersShowHideWidgetOpened"
bool GTUtilsOptionPanelSequenceView::isAnnotationParametersShowHideWidgetOpened(U2OpStatus &os) {
    QWidget *annotationsWidget = GTWidget::findWidget(os, "annotationsWidget");
    GT_CHECK_RESULT(NULL != annotationsWidget, "annotationsWidget is NULL", false);
    QWidget *annotationsParametersInnerWidget = GTWidget::findWidget(os, "annotationParametersInnerWidget", annotationsWidget);
    GT_CHECK_RESULT(NULL != annotationsParametersInnerWidget, "annotationsParametersInnerWidget is NULL", false);
    return annotationsParametersInnerWidget->isVisible();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openSearchAlgorithmShowHideWidget"
void GTUtilsOptionPanelSequenceView::openSearchAlgorithmShowHideWidget(U2OpStatus &os, bool open) {
    CHECK(open != isSearchAlgorithmShowHideWidgetOpened(os), );
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search algorithm"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openSearchInShowHideWidget"
void GTUtilsOptionPanelSequenceView::openSearchInShowHideWidget(U2OpStatus &os, bool open) {
    CHECK(open != isSearchInShowHideWidgetOpened(os), );
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search in"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openOtherSettingsShowHideWidget"
void GTUtilsOptionPanelSequenceView::openOtherSettingsShowHideWidget(U2OpStatus &os, bool open) {
    CHECK(open != isOtherSettingsShowHideWidgetOpened(os), );
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Other settings"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openSaveAnnotationToShowHideWidget"
void GTUtilsOptionPanelSequenceView::openSaveAnnotationToShowHideWidget(U2OpStatus &os, bool open) {
    CHECK(open != isSaveAnnotationToShowHideWidgetOpened(os), );
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Save annotation(s) to"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openAnnotationParametersShowHideWidget"
void GTUtilsOptionPanelSequenceView::openAnnotationParametersShowHideWidget(U2OpStatus &os, bool open) {
    CHECK(open != isAnnotationParametersShowHideWidgetOpened(os), );
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Annotation parameters"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleInputFromFilePattern"

void GTUtilsOptionPanelSequenceView::toggleInputFromFilePattern( U2OpStatus &os )
{
    QGroupBox *loadFromFile = qobject_cast<QGroupBox*>(GTWidget::findWidget(os, "loadFromFileGroupBox"));
    GTWidget::click(os, loadFromFile);
    //kinda hack for QGroupBox should be rewriten
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleSaveAnnotationsTo"

void GTUtilsOptionPanelSequenceView::toggleSaveAnnotationsTo(U2OpStatus &os){
    QLabel *obj = qobject_cast<QLabel*>(GTWidget::findWidget(os, "ArrowHeader_Save annotation(s) to"));
    GTWidget::click(os, obj);
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "enterPatternFromFile"

void GTUtilsOptionPanelSequenceView::enterPatternFromFile( U2OpStatus &os, QString filePathStr, QString fileName ){
    GTFileDialogUtils *ob = new GTFileDialogUtils(os, filePathStr, fileName, GTFileDialogUtils::Open);
    GTUtilsDialog::waitForDialog(os, ob);

    QToolButton *browse = qobject_cast<QToolButton*>(GTWidget::findWidget(os, "loadFromFileToolButton"));
    GTWidget::click(os, browse);
    GTGlobals::sleep(2500);
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setStrand"

void GTUtilsOptionPanelSequenceView::setStrand(U2OpStatus &os, QString strandStr) {
    QComboBox *strand = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "boxStrand", NULL, GTGlobals::FindOptions(false)));
    GT_CHECK(strand != NULL, "combobox is NULL");

    if(!strand->isVisible()){
        GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search in"));
    }
    GTComboBox::setIndexWithText(os, strand, strandStr);
    GTGlobals::sleep(2500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setRegionType"
void GTUtilsOptionPanelSequenceView::setRegionType(U2OpStatus &os, const QString &regionType) {
    openSearchInShowHideWidget(os);
    GTComboBox::setIndexWithText(os, GTWidget::findExactWidget<QComboBox *>(os, "boxRegion"), regionType, false);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setRegion"
void GTUtilsOptionPanelSequenceView::setRegion(U2OpStatus &os, int from, int to) {
    openSearchInShowHideWidget(os);
    GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "editStart"), QString::number(from));
    GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "editEnd"), QString::number(to));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "enterFilepathForSavingAnnotations"
void GTUtilsOptionPanelSequenceView::enterFilepathForSavingAnnotations(U2OpStatus &os, QString filepath) {
    QLineEdit *leFilePath = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "leNewTablePath"));
    GT_CHECK(leFilePath != NULL, "File path line edit is NULL");
    QDir().mkpath(QFileInfo(filepath).dir().absolutePath());
    GTLineEdit::setText(os, leFilePath, filepath);
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setAlgorithm"
void GTUtilsOptionPanelSequenceView::setAlgorithm( U2OpStatus &os, QString algorithm ){
    QComboBox *algoBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "boxAlgorithm"));
    GT_CHECK(algoBox != NULL, "algoBox is NULL");

    if(!algoBox->isVisible()){
        GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search algorithm"));
    }
    GTComboBox::setIndexWithText(os, algoBox, algorithm);
    GTGlobals::sleep(2500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getMatchPercentage"
int GTUtilsOptionPanelSequenceView::getMatchPercentage(U2OpStatus &os) {
    QSpinBox *spinMatchBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "spinBoxMatch"));
    GT_CHECK_RESULT(NULL != spinMatchBox, "Match percentage spinbox is NULL", -1);
    return spinMatchBox->value();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getRegionType"
QString GTUtilsOptionPanelSequenceView::getRegionType(U2OpStatus &os) {
    openSearchInShowHideWidget(os);
    QComboBox *cbRegionType = GTWidget::findExactWidget<QComboBox *>(os, "boxRegion");
    GT_CHECK_RESULT(NULL != cbRegionType, "Region type combobox is NULL", "");
    return cbRegionType->currentText();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setMatchPercentage"
QPair<int, int> GTUtilsOptionPanelSequenceView::getRegion(U2OpStatus &os) {
    openSearchInShowHideWidget(os);

    QPair<int, int> result;
    QLineEdit *leRegionStart = GTWidget::findExactWidget<QLineEdit *>(os, "editStart");
    QLineEdit *leRegionEnd = GTWidget::findExactWidget<QLineEdit *>(os, "editEnd");
    GT_CHECK_RESULT(NULL != leRegionStart, "Region start line edit is NULL", result);
    GT_CHECK_RESULT(NULL != leRegionEnd, "Region end line edit is NULL", result);

    bool ok = false;
    const int regionStart = leRegionStart->text().toInt(&ok);
    GT_CHECK_RESULT(ok, QString("Can't convert the string to int: %1").arg(leRegionStart->text()), result);
    const int regionEnd = leRegionEnd->text().toInt(&ok);
    GT_CHECK_RESULT(ok, QString("Can't convert the string to int: %1").arg(leRegionEnd->text()), result);

    return qMakePair(regionStart, regionEnd);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getHintText"
const QString GTUtilsOptionPanelSequenceView::getHintText(U2OpStatus &os) {
    QLabel *lblHint = GTWidget::findExactWidget<QLabel *>(os, "lblErrorMessage");
    GT_CHECK_RESULT(NULL != lblHint, "Hintlabel is NULL", "");
    CHECK(lblHint->isVisible(), "");
    return lblHint->text();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setMatchPercentage"
void GTUtilsOptionPanelSequenceView::setMatchPercentage( U2OpStatus &os, int percentage ){
    QSpinBox *spinMatchBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "spinBoxMatch"));

    GTSpinBox::setValue(os, spinMatchBox, percentage);
    GTGlobals::sleep(2500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setUsePatternName"
void GTUtilsOptionPanelSequenceView::setUsePatternName(U2OpStatus &os, bool setChecked) {
    openAnnotationParametersShowHideWidget(os);
    GTCheckBox::setChecked(os, GTWidget::findExactWidget<QCheckBox *>(os, "chbUsePatternNames"), setChecked);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setSearchInTranslation"
void GTUtilsOptionPanelSequenceView::setSearchInTranslation( U2OpStatus &os, bool inTranslation /* = true*/ ){
    QComboBox *searchIn = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "boxSeqTransl", NULL, GTGlobals::FindOptions(false)));
    GT_CHECK(searchIn != NULL, "combobox is NULL");

    if(!searchIn->isVisible()){
        GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search in"));
    }
    if(inTranslation){
        GTComboBox::setIndexWithText(os, searchIn, "Translation");
    }else{
        GTComboBox::setIndexWithText(os, searchIn, "Sequence");
    }
    GTGlobals::sleep(2500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setSearchInLocation"
void GTUtilsOptionPanelSequenceView::setSearchInLocation( U2OpStatus &os, QString locationStr ){
    QComboBox *region = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "boxRegion", NULL, GTGlobals::FindOptions(false)));
    GT_CHECK(region != NULL, "combobox is NULL");

    if(!region->isVisible()){
        GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search in"));
    }
    GTComboBox::setIndexWithText(os, region, locationStr, false);
    GTGlobals::sleep(2500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setSetMaxResults"
void GTUtilsOptionPanelSequenceView::setSetMaxResults( U2OpStatus &os, int maxResults ){
    QSpinBox *limit = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "boxMaxResult", NULL, GTGlobals::FindOptions(false)));
    GT_CHECK(limit != NULL, "spinbox is NULL");

    if(!limit->isVisible()){
        GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Other settings"));
    }
    GTSpinBox::setValue(os, limit, maxResults);
    GTGlobals::sleep(2500);
}

#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
