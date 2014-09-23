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

#include "GTUtilsMsaEditorSequenceArea.h"

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
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setStrand"

void GTUtilsOptionPanelSequenceView::setStrand(U2OpStatus &os, QString strandStr) {
    QComboBox *strand = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "boxStrand"));
    
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

#undef GT_CLASS_NAME

}
