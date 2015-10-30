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

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>

#include <U2Core/U2IdTypes.h>
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsOptionPanelMSA.h"
#include "api/GTBaseCompleter.h"
#include <primitives/GTComboBox.h>
#include <drivers/GTKeyboardDriver.h>
#include <primitives/GTTreeWidget.h>
#include <primitives/GTWidget.h>

namespace U2 {
using namespace HI;

QMap<GTUtilsOptionPanelMsa::Tabs, QString> GTUtilsOptionPanelMsa::initNames() {
    QMap<Tabs,QString> result;
    result.insert(General, "OP_MSA_GENERAL");
    result.insert(Highlighting, "OP_MSA_HIGHLIGHTING");
    result.insert(PairwiseAlignment, "OP_PAIRALIGN");
    result.insert(TreeSettings, "OP_MSA_ADD_TREE_WIDGET");
    result.insert(ExportConsensus, "OP_EXPORT_CONSENSUS");
    result.insert(Statistics, "OP_SEQ_STATISTICS_WIDGET");
    return result;
}

QMap<GTUtilsOptionPanelMsa::Tabs, QString> GTUtilsOptionPanelMsa::initInnerWidgetNames() {
    QMap<Tabs, QString> result;
    result.insert(General, "MsaGeneralTab");
    result.insert(Highlighting, "HighlightingOptionsPanelWidget");
    result.insert(PairwiseAlignment, "PairwiseAlignmentOptionsPanelWidget");
    result.insert(TreeSettings, "AddTreeWidget");
    result.insert(ExportConsensus, "ExportConsensusWidget");
    result.insert(Statistics, "SequenceStatisticsOptionsPanelTab");
    return result;
}
const QMap<GTUtilsOptionPanelMsa::Tabs, QString> GTUtilsOptionPanelMsa::tabsNames = initNames();
const QMap<GTUtilsOptionPanelMsa::Tabs, QString> GTUtilsOptionPanelMsa::innerWidgetNames = initInnerWidgetNames();

#define GT_CLASS_NAME "GTUtilsOptionPanelMSA"

#define GT_METHOD_NAME "toggleTab"
void GTUtilsOptionPanelMsa::toggleTab(U2OpStatus &os, GTUtilsOptionPanelMsa::Tabs tab) {
    GTWidget::click(os, GTWidget::findWidget(os, tabsNames[tab]));
    GTGlobals::sleep(200);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openTab"
void GTUtilsOptionPanelMsa::openTab(U2OpStatus &os, Tabs tab) {
    if (!isTabOpened(os, tab)) {
        toggleTab(os, tab);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "closeTab"
void GTUtilsOptionPanelMsa::closeTab(U2OpStatus &os, Tabs tab) {
    if (isTabOpened(os, tab)) {
        toggleTab(os, tab);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isTabOpened"
bool GTUtilsOptionPanelMsa::isTabOpened(U2OpStatus &os, Tabs tab) {
    GTGlobals::FindOptions options;
    options.failIfNull = false;
    QWidget *innerTabWidget = GTWidget::findWidget(os, innerWidgetNames[tab], NULL, options);
    return NULL != innerTabWidget && innerTabWidget->isVisible();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addReference"
void GTUtilsOptionPanelMsa::addReference(U2OpStatus &os, QString seqName, AddRefMethod method){
    GT_CHECK(!seqName.isEmpty(), "sequence name is empty");
    //Option panel should be opned to use this method
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);

    GT_CHECK(nameList.contains(seqName), QString("sequence with name %1 not found").arg(seqName));

    switch (method)
    {
    case Button:
        GTUtilsMSAEditorSequenceArea::selectSequence(os, seqName);
        GTWidget::click(os, GTWidget::findWidget(os, "addSeq"));
        break;
    case Completer:
        QWidget* sequenceLineEdit = GTWidget::findWidget(os, "sequenceLineEdit");
        GTWidget::click(os, sequenceLineEdit);
        GTKeyboardDriver::keyClick(os, seqName.at(0).toLatin1());
        GTGlobals::sleep(200);
        QTreeWidget* completer = sequenceLineEdit->findChild<QTreeWidget*>();
        GT_CHECK(completer != NULL, "auto completer widget not found");
        GTBaseCompleter::click(os, completer, seqName);
        break;
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "removeReference"
void GTUtilsOptionPanelMsa::removeReference(U2OpStatus &os){
    GTWidget::click(os, GTWidget::findWidget(os, "deleteSeq"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getReference"
QString GTUtilsOptionPanelMsa::getReference(U2OpStatus &os) {
    openTab(os, General);
    QLineEdit *leReference = GTWidget::findExactWidget<QLineEdit *>(os, "sequenceLineEdit");
    GT_CHECK_RESULT(NULL != leReference, "Reference sequence name lineedit is NULL", QString());
    return leReference->text();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getLength"
int GTUtilsOptionPanelMsa::getLength(U2OpStatus &os){
    QLabel* alignmentLengthLabel = qobject_cast<QLabel*>(GTWidget::findWidget(os, "alignmentLength"));
    GT_CHECK_RESULT(alignmentLengthLabel != NULL, "alignmentLengthLabel not found", -1);
    bool ok;
    int result = alignmentLengthLabel->text().toInt(&ok);
    GT_CHECK_RESULT(ok == true, "label text is not int", -1);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getHeight"
int GTUtilsOptionPanelMsa::getHeight(U2OpStatus &os){
    QLabel* alignmentHeightLabel = qobject_cast<QLabel*>(GTWidget::findWidget(os, "alignmentHeight"));
    GT_CHECK_RESULT(alignmentHeightLabel != NULL, "alignmentHeightLabel not found", -1);
    bool ok;
    int result = alignmentHeightLabel->text().toInt(&ok);
    GT_CHECK_RESULT(ok == true, "label text is not int", -1);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setColorScheme"
void GTUtilsOptionPanelMsa::setColorScheme(U2OpStatus &os, const QString &colorSchemeName) {
    openTab(os, Highlighting);
    GTComboBox::setIndexWithText(os, GTWidget::findExactWidget<QComboBox *>(os, "colorScheme"), colorSchemeName);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getColorScheme"
QString GTUtilsOptionPanelMsa::getColorScheme(U2OpStatus &os) {
    openTab(os, Highlighting);
    QComboBox *colorScheme = GTWidget::findExactWidget<QComboBox *>(os, "colorScheme");
    GT_CHECK_RESULT(NULL != colorScheme, "ColorSCheme combobox is NULL", "");
    return colorScheme->currentText();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addFirstSeqToPA"
void GTUtilsOptionPanelMsa::addFirstSeqToPA(U2OpStatus &os, QString seqName, AddRefMethod method){
    addSeqToPA(os, seqName, method, 1);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addSecondSeqToPA"
void GTUtilsOptionPanelMsa::addSecondSeqToPA(U2OpStatus &os, QString seqName, AddRefMethod method){
    addSeqToPA(os, seqName, method, 2);
}
#undef GT_METHOD_NAME

QString GTUtilsOptionPanelMsa::getSeqFromPAlineEdit(U2OpStatus &os, int num){
    QLineEdit* le = qobject_cast<QLineEdit*>(getWidget(os, "sequenceLineEdit", num));
    return le->text();
}

#define GT_METHOD_NAME "addSeqToPA"
void GTUtilsOptionPanelMsa::addSeqToPA(U2OpStatus &os, QString seqName, AddRefMethod method, int number){
    GT_CHECK(number == 1 || number == 2, "number must be 1 or 2");
    GT_CHECK(!seqName.isEmpty(), "sequence name is empty");
    //Option panel should be opned to use this method
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);

    GT_CHECK(nameList.contains(seqName), QString("sequence with name %1 not found").arg(seqName));

    switch (method)
    {
    case Button:
        GTUtilsMSAEditorSequenceArea::selectSequence(os, seqName);
        GTWidget::click(os, getAddButton(os, number));
        break;
    case Completer:
        QWidget* sequenceLineEdit = getSeqLineEdit(os, number);
        GTWidget::click(os, sequenceLineEdit);
        GTKeyboardDriver::keyClick(os, seqName.at(0).toLatin1());
        GTGlobals::sleep(200);
        QTreeWidget* completer = sequenceLineEdit->findChild<QTreeWidget*>();
        GT_CHECK(completer != NULL, "auto completer widget not found");
        GTBaseCompleter::click(os, completer, seqName);
        break;
    }
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "getAddButton"
QToolButton* GTUtilsOptionPanelMsa::getAddButton(U2OpStatus &os, int number){
    QToolButton* result = qobject_cast<QToolButton*>(getWidget(os, "addSeq", number));
    GT_CHECK_RESULT(result != NULL, "toolbutton is NULL", NULL);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getDeleteButton"
QToolButton* GTUtilsOptionPanelMsa::getDeleteButton(U2OpStatus &os, int number){
    QToolButton* result = qobject_cast<QToolButton*>(getWidget(os, "deleteSeq", number));
    GT_CHECK_RESULT(result != NULL, "toolbutton is NULL", NULL);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getAlignButton"
QPushButton *GTUtilsOptionPanelMsa::getAlignButton(U2OpStatus &os) {
    // TODO: ensure that the Pairwise alignment" tab is opened.
    return GTWidget::findExactWidget<QPushButton *>(os, "alignButton");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setPairwiseAlignmentAlgorithm"
void GTUtilsOptionPanelMsa::setPairwiseAlignmentAlgorithm(U2OpStatus &os, const QString &algorithm) {
    // TODO: ensure that the Pairwise alignment" tab is opened.
    GTComboBox::setIndexWithText(os, GTWidget::findExactWidget<QComboBox*>(os, "algorithmListComboBox"), algorithm);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSeqLineEdit"
QLineEdit* GTUtilsOptionPanelMsa::getSeqLineEdit(U2OpStatus &os, int number){
    QLineEdit* result = qobject_cast<QLineEdit*>(getWidget(os, "sequenceLineEdit", number));
    GT_CHECK_RESULT(result != NULL, "sequenceLineEdit is NULL", NULL);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "agetWidget"
QWidget* GTUtilsOptionPanelMsa::getWidget(U2OpStatus &os, const QString& widgetName, int number){
    QWidget* sequenceContainerWidget = GTWidget::findWidget(os, "sequenceContainerWidget");
    GT_CHECK_RESULT(sequenceContainerWidget != NULL, "sequenceContainerWidget not found", NULL);
    QList<QWidget*> widgetList = sequenceContainerWidget->findChildren<QWidget*>(widgetName);
    GT_CHECK_RESULT(widgetList.count() == 2, QString("unexpected number of widgets: %1").arg(widgetList.count()), NULL);
    QWidget* w1 = widgetList[0];
    QWidget* w2 = widgetList[1];
    int y1 = w1->mapToGlobal(w1->rect().center()).y();
    int y2 = w2->mapToGlobal(w2->rect().center()).y();
    GT_CHECK_RESULT(y1 != y2, "coordinates are unexpectidly equal", NULL);

    if(number == 1){
        if(y1 < y2){
            return w1;
        }else{
            return w2;
        }
    }else if(number == 2){
        if(y1 < y2){
            return w2;
        }else{
            return w1;
        }
    }else{
        GT_CHECK_RESULT(false, "number should be 1 or 2", NULL);
    }
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME
}
