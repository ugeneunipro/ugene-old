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

#include <U2Gui/MainWindow.h>

#include <U2View/MSAEditorConsensusArea.h>
#include <U2View/MSAEditorNameList.h>
#include <U2View/MSAEditorOverviewArea.h>
#include <U2View/MSAGraphOverview.h>
#include <U2View/MSASimpleOverview.h>

#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditor.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsOptionPanelMSA.h"
#include <drivers/GTKeyboardDriver.h>
#include <drivers/GTMouseDriver.h>
#include "api/GTMSAEditorStatusWidget.h"
#include <primitives/GTToolbar.h>
#include "primitives/PopupChooser.h"

namespace U2 {
using namespace HI;

#define GT_CLASS_NAME "GTUtilsMsaEditor"

#define GT_METHOD_NAME "getGraphOverviewTopLeftPixelColor"
QColor GTUtilsMsaEditor::getGraphOverviewPixelColor(U2OpStatus &os, const QPoint &point) {
    return GTWidget::getColor(os, getGraphOverview(os), point);
}

QColor GTUtilsMsaEditor::getSimpleOverviewPixelColor(U2OpStatus &os, const QPoint &point) {
    return GTWidget::getColor(os, getSimpleOverview(os), point);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getEditor"
MSAEditor * GTUtilsMsaEditor::getEditor(U2OpStatus &os) {
    MSAEditorUI *editorUi = getEditorUi(os);
    CHECK_OP(os, NULL);
    return editorUi->getEditor();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getEditorUi"
MSAEditorUI * GTUtilsMsaEditor::getEditorUi(U2OpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);
    return activeWindow->findChild<MSAEditorUI *>();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getGraphOverview"
MSAGraphOverview * GTUtilsMsaEditor::getGraphOverview(U2OpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);

    MSAGraphOverview *result = GTWidget::findExactWidget<MSAGraphOverview *>(os, MSAEditorOverviewArea::OVERVIEW_AREA_OBJECT_NAME + QString("_graph"), activeWindow);
    GT_CHECK_RESULT(NULL != result, "MSAGraphOverview is not found", NULL);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSimpleOverview"
MSASimpleOverview * GTUtilsMsaEditor::getSimpleOverview(U2OpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);

    MSASimpleOverview *result = GTWidget::findExactWidget<MSASimpleOverview *>(os, MSAEditorOverviewArea::OVERVIEW_AREA_OBJECT_NAME + QString("_simple"), activeWindow);
    GT_CHECK_RESULT(NULL != result, "MSASimpleOverview is not found", NULL);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getTreeView"
MSAEditorTreeViewerUI * GTUtilsMsaEditor::getTreeView(U2OpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);
    return GTWidget::findExactWidget<MSAEditorTreeViewerUI *>(os, "treeView", activeWindow);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getNameListArea"
MSAEditorNameList * GTUtilsMsaEditor::getNameListArea(U2OpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);

    MSAEditorNameList *result = GTWidget::findExactWidget<MSAEditorNameList *>(os, "msa_editor_name_list", activeWindow);
    GT_CHECK_RESULT(NULL != result, "MSAGraphOverview is not found", NULL);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getConsensusArea"
MSAEditorConsensusArea * GTUtilsMsaEditor::getConsensusArea(U2OpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);
    return GTWidget::findExactWidget<MSAEditorConsensusArea *>(os, "consArea", activeWindow);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSequenceNameRect"
MSAEditorSequenceArea * GTUtilsMsaEditor::getSequenceArea(U2OpStatus &os) {
    return GTUtilsMSAEditorSequenceArea::getSequenceArea(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSequenceNameRect"
QRect GTUtilsMsaEditor::getSequenceNameRect(U2OpStatus &os, const QString &sequenceName) {
    Q_UNUSED(os);
    MSAEditorNameList *nameList = getNameListArea(os);
    GT_CHECK_RESULT(NULL != nameList, "MSAEditorNameList not found", QRect());

    const int rowHeight = GTUtilsMSAEditorSequenceArea::getRowHeight(os);
    const QStringList names = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os);
    const int rowNumber = names.indexOf(sequenceName);
    GT_CHECK_RESULT(0 <= rowNumber, QString("Sequence '%1' not found").arg(sequenceName), QRect());

    return QRect(nameList->mapToGlobal(QPoint(0, rowHeight * rowNumber)), nameList->mapToGlobal(QPoint(nameList->width(), rowHeight * (rowNumber + 1))));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getColumnHeaderRect"
QRect GTUtilsMsaEditor::getColumnHeaderRect(U2OpStatus &os, int column) {
    MSAEditorConsensusArea *consensusArea = getConsensusArea(os);
    GT_CHECK_RESULT(NULL != consensusArea, "Consensus area is NULL", QRect());
    MSAEditorSequenceArea *sequenceArea = getSequenceArea(os);
    GT_CHECK_RESULT(NULL != sequenceArea, "Sequence area is NULL", QRect());
    MSAEditor *editor = getEditor(os);
    GT_CHECK_RESULT(NULL != editor, "MSA Editor is NULL", QRect());

    return QRect(sequenceArea->getBaseXRange(column, false).startPos, consensusArea->geometry().top(), editor->getColumnWidth(), consensusArea->height());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "replaceSequence"
void GTUtilsMsaEditor::replaceSequence(U2OpStatus &os, const QString &sequenceToReplace, int targetPosition) {
    clickSequenceName(os, sequenceToReplace);

    targetPosition = qMax(0, qMin(getSequencesCount(os) - 1, targetPosition));
    const QString targetSequenceName = GTUtilsMSAEditorSequenceArea::getNameList(os)[targetPosition];

    const QPoint dragFrom = getSequenceNameRect(os, sequenceToReplace).center();
    const QPoint dragTo = getSequenceNameRect(os, targetSequenceName).center();

    GTMouseDriver::moveTo(os, dragFrom);
    GTMouseDriver::press(os);
    GTMouseDriver::moveTo(os, dragTo);
    GTMouseDriver::release(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "removeColumn"
void GTUtilsMsaEditor::removeColumn(U2OpStatus &os, int column) {
    clickColumn(os, column);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickSequenceName"
void GTUtilsMsaEditor::clickSequenceName(U2OpStatus &os, const QString &sequenceName, Qt::MouseButton mouseButton) {
    const QRect sequenceNameRect = getSequenceNameRect(os, sequenceName);
    GTMouseDriver::moveTo(os, sequenceNameRect.center());
    GTMouseDriver::click(os, mouseButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickColumn"
void GTUtilsMsaEditor::clickColumn(U2OpStatus &os, int column, Qt::MouseButton mouseButton) {
    MSAEditorConsensusArea *consensusArea = getConsensusArea(os);
    GT_CHECK(NULL != consensusArea, "Consensus area is NULL");

    GTUtilsMSAEditorSequenceArea::scrollToPosition(os, QPoint(column, 1));
    const QRect columnHeaderRect = getColumnHeaderRect(os, column);
    GTWidget::click(os, consensusArea, mouseButton, columnHeaderRect.center());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getReferenceSequenceName"
QString GTUtilsMsaEditor::getReferenceSequenceName(U2OpStatus &os) {
    return GTUtilsOptionPanelMsa::getReference(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleCollapsingMode"
void GTUtilsMsaEditor::toggleCollapsingMode(U2OpStatus &os) {
    Q_UNUSED(os);
    GTWidget::click(os, GTToolbar::getWidgetForActionTooltip(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "Switch on/off collapsing"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isSequenceCollapsed"
bool GTUtilsMsaEditor::isSequenceCollapsed(U2OpStatus &os, const QString &seqName){
    QStringList names = GTUtilsMSAEditorSequenceArea::getNameList(os);
    GT_CHECK_RESULT(names.contains(seqName), "sequence " + seqName + " not found in name list", false);
    QStringList visiablenames = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os);

    return !visiablenames.contains(seqName);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleCollapsingMode"
void GTUtilsMsaEditor::toggleCollapsingGroup(U2OpStatus &os, const QString &groupName) {
    Q_UNUSED(os);

    const QRect sequenceNameRect = getSequenceNameRect(os, groupName);
    QPoint magicExpandButtonOffset;
#ifdef Q_OS_WIN
    magicExpandButtonOffset = QPoint(15, 10);
#else
    magicExpandButtonOffset = QPoint(15, 5);
#endif
    GTMouseDriver::moveTo(os, sequenceNameRect.topLeft() + magicExpandButtonOffset);
    GTMouseDriver::click(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSequencesCount"
int GTUtilsMsaEditor::getSequencesCount(U2OpStatus &os) {
    QWidget *statusWidget = GTWidget::findWidget(os, "msa_editor_status_bar");
    return GTMSAEditorStatusWidget::getSequencesCount(os, statusWidget);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "undo"
void GTUtilsMsaEditor::undo(U2OpStatus &os) {
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "msa_action_undo"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "redo"
void GTUtilsMsaEditor::redo(U2OpStatus &os) {
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "msa_action_redo"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setReference"
void GTUtilsMsaEditor::setReference(U2OpStatus &os, const QString &sequenceName) {
    GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Set this sequence as reference", GTGlobals::UseMouse));
    clickSequenceName(os, sequenceName, Qt::RightButton);
    GTGlobals::sleep(100);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
