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

#include "GTUtilsOptionPanelSequenceView.h"

#include <QtCore/qglobal.h>
#include <QtCore/QDir>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QTreeWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QLabel>
#else
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>
#endif

#include "api/GTWidget.h"
#include "api/GTTextEdit.h"
#include "api/GTClipboard.h"
#include "api/GTComboBox.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTSpinBox.h"

#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsTaskTreeView.h"

namespace U2{

QMap<GTUtilsOptionPanelSequenceView::Tabs, QString> GTUtilsOptionPanelSequenceView::initNames(){
    QMap<Tabs,QString> result;
    result.insert(Search, "OP_FIND_PATTERN");
    result.insert(AnnotationsHighlighting, "OP_ANNOT_HIGHLIGHT");
    result.insert(Statistics, "OP_PAIRALIGN");
    return result;
}

const QMap<GTUtilsOptionPanelSequenceView::Tabs,QString> GTUtilsOptionPanelSequenceView::tabsNames = initNames();

#define GT_CLASS_NAME "GTUtilsOptionPanelSequenceView"

#define GT_METHOD_NAME "enterPattern"

void GTUtilsOptionPanelSequenceView::enterPattern( U2OpStatus &os, QString pattern ){
    QTextEdit *patternEdit = qobject_cast<QTextEdit*>(GTWidget::findWidget(os, "textPattern"));
    GTWidget::click(os, patternEdit);

    GTTextEdit::clear(os, patternEdit);
    GTTextEdit::setText(os, patternEdit, pattern);

    GTGlobals::sleep(3000);
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openTab"

void GTUtilsOptionPanelSequenceView::openTab(U2OpStatus &os, Tabs tab){
    GTWidget::click(os, GTWidget::findWidget(os, tabsNames[tab]));
    GTGlobals::sleep(200);
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkResultsText"

bool GTUtilsOptionPanelSequenceView::checkResultsText(U2OpStatus &os, QString expectedText) {
    QLabel *label = qobject_cast<QLabel*>(GTWidget::findWidget(os, "resultLabel"));
    return label->text() == expectedText;
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
    QWidget *saveAnnotationsToInnerWidget = GTWidget::findWidget(os, "saveAnnotationsToWidget", annotationsWidget);
    GT_CHECK_RESULT(NULL != saveAnnotationsToInnerWidget, "saveAnnotationsToInnerWidget is NULL", false);
    return saveAnnotationsToInnerWidget->isVisible();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isAnnotationParametersShowHideWidgetOpened"
bool GTUtilsOptionPanelSequenceView::isAnnotationParametersShowHideWidgetOpened(U2OpStatus &os) {
    QWidget *annotationsWidget = GTWidget::findWidget(os, "annotationsWidget");
    GT_CHECK_RESULT(NULL != annotationsWidget, "annotationsWidget is NULL", false);
    QWidget *annotationsParametersInnerWidget = GTWidget::findWidget(os, "annotationParametersWidget", annotationsWidget);
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

#define GT_METHOD_NAME "enterFilepathForSavingAnnotations"
void GTUtilsOptionPanelSequenceView::enterFilepathForSavingAnnotations(U2OpStatus &os, QString filepath) {
    QLineEdit *leFilePath = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "newFilePath"));
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

#define GT_METHOD_NAME "setMatchPercentage"
void GTUtilsOptionPanelSequenceView::setMatchPercentage( U2OpStatus &os, int percentage ){
    QSpinBox *spinMatchBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "spinBoxMatch"));

    GTSpinBox::setValue(os, spinMatchBox, percentage);
    GTGlobals::sleep(2500);
}

#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
