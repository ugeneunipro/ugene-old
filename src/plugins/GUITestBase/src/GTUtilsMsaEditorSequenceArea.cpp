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

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorDataList.h>
#include <U2Core/AppContext.h>

#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTMSAEditorStatusWidget.h"
#include "api/GTWidget.h"
#include "runnables/ugene/corelibs/U2Gui/util/RenameSequenceFiller.h"
#include "runnables/qt/PopupChooser.h"
#include <QMainWindow>


namespace U2 {
const QString GTUtilsMSAEditorSequenceArea::highlightningColorName = "#9999cc";

#define GT_CLASS_NAME "GTUtilsMSAEditorSequenceArea"

#define GT_METHOD_NAME "moveTo"
void GTUtilsMSAEditorSequenceArea::moveTo(U2OpStatus &os, const QPoint &p)
{
    QPoint convP = convertCoordinates(os,p);

    GTMouseDriver::moveTo(os, convP);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "convertCoordinates"
QPoint GTUtilsMSAEditorSequenceArea::convertCoordinates(U2OpStatus &os, const QPoint p){
    QWidget* activeWindow = GTUtilsMdi::activeWindow(os);
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area", activeWindow));
    GT_CHECK_RESULT(msaEditArea != NULL, "MsaEditorSequenceArea not found",QPoint());

    QWidget *msaOffsetLeft = GTWidget::findWidget(os, "msa_editor_offsets_view_widget_left", activeWindow);
    GT_CHECK_RESULT(msaOffsetLeft != NULL, "MsaOffset Left not found",QPoint());

    QPoint shift = msaOffsetLeft->mapToGlobal(QPoint(0, 0));
    if (msaOffsetLeft->isVisible()) {
        shift = msaOffsetLeft->mapToGlobal(QPoint(msaOffsetLeft->rect().right(), 0));
    }

    int posX = msaEditArea->getXByColumnNum(p.x());
    int posY = msaEditArea->getYBySequenceNum(p.y());

    return shift+QPoint(posX, posY);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectArea"
void GTUtilsMSAEditorSequenceArea::selectArea(U2OpStatus &os, QPoint p1, QPoint p2)
{
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area", GTUtilsMdi::activeWindow(os)));
    GT_CHECK(msaEditArea != NULL, "MsaEditorSequenceArea not found");

    p1.rx() = p1.x()==-1 ? msaEditArea->getNumVisibleBases(true)-1 : p1.x();
    p2.rx() = p2.x()==-1 ? msaEditArea->getNumVisibleBases(true)-1 : p2.x();
    p1.ry() = p1.y()==-1 ? msaEditArea->getNumVisibleSequences(true)-1 : p1.y();
    p2.ry() = p2.y()==-1 ? msaEditArea->getNumVisibleSequences(true)-1 : p2.y();

    moveTo(os, p1);
    GTMouseDriver::press(os);
    GTMouseDriver::moveTo(os,convertCoordinates(os,p2));
    GTMouseDriver::release(os);
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "click"
void GTUtilsMSAEditorSequenceArea::click(U2OpStatus &os, QPoint p) {
    selectArea(os, p, p);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "scrollToPosition"
void GTUtilsMSAEditorSequenceArea::scrollToPosition(U2OpStatus &os, const QPoint &position) {
    MSAEditorSequenceArea *msaSeqArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area", GTUtilsMdi::activeWindow(os)));
    GT_CHECK(NULL != msaSeqArea, "MSA Editor sequence area is not found");
    GT_CHECK(msaSeqArea->isInRange(position), "Position is out of range");

    // scroll down
    GScrollBar* vBar = msaSeqArea->getVBar();
    GT_CHECK(NULL != vBar, "Vertical scroll bar is not found");

    QStyleOptionSlider vScrollBarOptions;
    vScrollBarOptions.initFrom(vBar);

    while (!msaSeqArea->isSeqVisible(position.y(), false)) {
        const QRect sliderSpaceRect = vBar->style()->subControlRect(QStyle::CC_ScrollBar, &vScrollBarOptions, QStyle::SC_ScrollBarGroove, vBar);
        const QPoint bottomEdge(sliderSpaceRect.width() / 2, sliderSpaceRect.y() + sliderSpaceRect.height());

        GTMouseDriver::moveTo(os, vBar->mapToGlobal(bottomEdge) - QPoint(0, 1));
        GTMouseDriver::click(os);
    }

    // scroll right
    GScrollBar* hBar = msaSeqArea->getHBar();
    GT_CHECK(NULL != hBar, "Horisontal scroll bar is not found");

    QStyleOptionSlider hScrollBarOptions;
    hScrollBarOptions.initFrom(hBar);

    while (!msaSeqArea->isPosVisible(position.x(), false)) {
        const QRect sliderSpaceRect = hBar->style()->subControlRect(QStyle::CC_ScrollBar, &hScrollBarOptions, QStyle::SC_ScrollBarGroove, hBar);
        const QPoint rightEdge(sliderSpaceRect.x() + sliderSpaceRect.width(), sliderSpaceRect.height() / 2);

        GTMouseDriver::moveTo(os, hBar->mapToGlobal(rightEdge) - QPoint(1, 0));
        GTMouseDriver::click(os);
    }

    SAFE_POINT(msaSeqArea->isVisible(position, false), "The position is still invisible after scrolling", );
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickToPosition"
void GTUtilsMSAEditorSequenceArea::clickToPosition(U2OpStatus &os, const QPoint &position) {
    MSAEditorSequenceArea *msaSeqArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area", GTUtilsMdi::activeWindow(os)));
    GT_CHECK(NULL != msaSeqArea, "MSA Editor sequence area is not found");
    GT_CHECK(msaSeqArea->isInRange(position), "Position is out of range");

    scrollToPosition(os, position);
    const QPoint visibleStart(msaSeqArea->getFirstVisibleBase(), msaSeqArea->getFirstVisibleSequence());
    click(os, position - visibleStart);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkSelectedRect"
void GTUtilsMSAEditorSequenceArea::checkSelectedRect(U2OpStatus &os, const QRect &expectedRect)
{
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR(msaEditArea != NULL, "MsaEditorSequenceArea not found");

    QRect msaEditRegion = msaEditArea->getSelection().getRect();
    CHECK_SET_ERR(expectedRect == msaEditRegion, "Unexpected selection region");
}
#undef GT_METHOD_NAME
#define GT_METHOD_NAME "checkSorted"
void GTUtilsMSAEditorSequenceArea::checkSorted(U2OpStatus &os, bool sortedState) {

    QStringList names = getNameList(os);

    QStringList sortedNames = names;
    qSort(sortedNames);

    bool sorted = (sortedNames == names);
    GT_CHECK(sorted == sortedState, "Sorted state differs from needed sorted state");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getNameList"
QStringList GTUtilsMSAEditorSequenceArea::getNameList(U2OpStatus &os) {

    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    MSAEditor* editor = mw->findChild<MSAEditor*>();
    CHECK_SET_ERR_RESULT(editor != NULL, "MsaEditor not found", QStringList());

    QStringList result = editor->getMSAObject()->getMAlignment().getRowNames();

    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getVisibaleNames"
QStringList GTUtilsMSAEditorSequenceArea::getVisibaleNames(U2OpStatus &os){
    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    MSAEditor* editor = mw->findChild<MSAEditor*>();
    CHECK_SET_ERR_RESULT(editor != NULL, "MsaEditor not found", QStringList());

    MSAEditorSequenceArea* seqArea = editor->getUI()->getSequenceArea();
    CHECK_SET_ERR_RESULT(NULL != seqArea, "MSA Editor sequence area is NULL", QStringList());


    int startSeq = seqArea->getFirstVisibleSequence();
    int lastSeq = seqArea->getLastVisibleSequence(true);
    QVector<U2Region> rows;
    editor->getUI()->getCollapseModel()->getVisibleRows(startSeq, lastSeq, rows);

    QStringList visiableRowNames;
    foreach(U2Region region, rows){
        for(int x = region.startPos; x < region.endPos(); x++)
            visiableRowNames.append(editor->getMSAObject()->getRow(x).getName());
    }

    return visiableRowNames;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSimilarityValue"
QString GTUtilsMSAEditorSequenceArea::getSimilarityValue(U2OpStatus &os, int row){
    //bad sulution, but better then nothing
    MSAEditorSimilarityColumn* simCol = dynamic_cast<MSAEditorSimilarityColumn*>(GTWidget::findWidget(os, "msa_editor_similarity_column"));
    GT_CHECK_RESULT(simCol != NULL, "SimilarityColumn is NULL", "");

    return simCol->getTextForRow(row);

}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getLeftOffset"
int GTUtilsMSAEditorSequenceArea::getLeftOffset(U2OpStatus &os)
{
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR_RESULT(msaEditArea != NULL, "MsaEditorSequenceArea not found", -1);

    return msaEditArea->getFirstVisibleBase() + 1; // тут не уверен, есть еще класс MSAEditorOffsetsViewWidget (файл MSAEditorOffsetsViewWidget.h)
                                                   // мне кажется более правильно будет от туда офсет вытащить.
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getLeftOffset"
int GTUtilsMSAEditorSequenceArea::getRightOffset(U2OpStatus &os)
{
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR_RESULT(msaEditArea != NULL, "MsaEditorSequenceArea not found", -1);

    return msaEditArea->getLastVisibleBase(true, true) + 1; // тут такая же фигня как getLeftOffset()
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getLength"
int GTUtilsMSAEditorSequenceArea::getLength(U2OpStatus &os) {

    QWidget *statusWidget = GTWidget::findWidget(os, "msa_editor_status_bar");
    return GTMSAEditorStatusWidget::length(os, statusWidget);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "offsetsVisible"
bool GTUtilsMSAEditorSequenceArea::offsetsVisible(U2OpStatus &os) {

    QWidget *leftOffsetWidget = GTWidget::findWidget(os, "msa_editor_offsets_view_widget_left");
    QWidget *rightOffsetWidget = GTWidget::findWidget(os, "msa_editor_offsets_view_widget_right");

    GT_CHECK_RESULT((leftOffsetWidget != NULL) && (rightOffsetWidget != NULL), "offset widgets are NULL", false);
    GT_CHECK_RESULT(leftOffsetWidget->isVisible() == rightOffsetWidget->isVisible(), "offset widget visibility states are not the same", false);

    return leftOffsetWidget->isVisible();
}
#undef GT_METHOD_NAME
#define GT_METHOD_NAME "checkConsensus"
void GTUtilsMSAEditorSequenceArea::checkConsensus(U2OpStatus &os, QString cons){
    QWidget *consArea = GTWidget::findWidget(os,"consArea");
    CHECK_SET_ERR(consArea!=NULL,"consArea is NULL");

    QObject *parent = consArea->findChild<QObject*>("parent");
    CHECK_SET_ERR(parent!=NULL,"parent is NULL");

    QObject *child = parent->findChild<QObject*>();
    CHECK_SET_ERR(child!=NULL,"child is NULL");
    CHECK_SET_ERR(child->objectName()==cons,"Wrong consensus. Currens consensus is  "+child->objectName());
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectSequence"
void GTUtilsMSAEditorSequenceArea::selectSequence(U2OpStatus &os, QString seqName) {
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>
            (GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR(msaEditArea != NULL, "MsaEditorSequenceArea not found");

    QStringList names = getVisibaleNames(os);
    int row = 0;
    while (names[row] != seqName) {
        row++;
    }
    click(os, QPoint(-5, row));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isSequenceSelected"
bool GTUtilsMSAEditorSequenceArea::isSequenceSelected(U2OpStatus &os, QString seqName) {
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>
            (GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR_RESULT(msaEditArea != NULL, "MsaEditorSequenceArea not found", false);

    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    MSAEditor* editor = mw->findChild<MSAEditor*>();
    CHECK_SET_ERR_RESULT(editor != NULL, "MsaEditor not found", false);
//Seq names are drawn on widget, so this hack is needed
    U2Region selectedRowsRegion = msaEditArea->getSelectedRows();
    QStringList selectedRowNames;
    for(int x = selectedRowsRegion.startPos; x < selectedRowsRegion.endPos(); x++)
        selectedRowNames.append(editor->getMSAObject()->getRow(x).getName());

    if (selectedRowNames.contains(seqName)) {
        return true;
    }
    return false;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME
int GTUtilsMSAEditorSequenceArea::getSelectedSequencesNum(U2OpStatus &os) {
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>
        (GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR_RESULT(msaEditArea != NULL, "MsaEditorSequenceArea not found", 0);

    return msaEditArea->getSelectedRows().length;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isSequenceVisible"
bool GTUtilsMSAEditorSequenceArea::isSequenceVisible(U2OpStatus &os, QString seqName) {    
    QStringList visiableRowNames = getVisibaleNames(os);
    return visiableRowNames.contains(seqName);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectColumnInConsensus"
void GTUtilsMSAEditorSequenceArea::selectColumnInConsensus( U2OpStatus &os, int columnNumber ) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow( os );
    const MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea *>(
        GTWidget::findWidget( os, "msa_editor_sequence_area", activeWindow ) );
    GT_CHECK_RESULT( NULL != msaEditArea, "MsaEditorSequenceArea not found", );

    const QWidget *msaOffsetLeft = GTWidget::findWidget( os, "msa_editor_offsets_view_widget_left",
        activeWindow );
    GT_CHECK_RESULT( NULL != msaOffsetLeft, "MsaOffset Left not found", );

    QPoint shift = msaOffsetLeft->mapToGlobal( QPoint( 0, 0 ) );
    if ( msaOffsetLeft->isVisible( ) ) {
        shift = msaOffsetLeft->mapToGlobal( QPoint( msaOffsetLeft->rect( ).right( ), 0 ) );
    }

    const int posX = msaEditArea->getXByColumnNum( columnNumber ) + shift.x( );

    QWidget *consArea = GTWidget::findWidget( os,"consArea" );
    CHECK_SET_ERR( NULL != consArea,"consArea is NULL" );

    const int posY = consArea->mapToGlobal( consArea->rect( ).center( ) ).y( );
    GTMouseDriver::moveTo( os, QPoint( posX, posY ) );
    GTMouseDriver::click( os );
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "hasAminoAlphabet"
bool GTUtilsMSAEditorSequenceArea::hasAminoAlphabet(U2OpStatus &os)
{
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR_RESULT(msaEditArea != NULL, "MsaEditorSequenceArea not found", false);

    return msaEditArea->hasAminoAlphabet();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isSequenceHightighted"
bool GTUtilsMSAEditorSequenceArea::isSequenceHightighted(U2OpStatus &os, const QString &seqName){
    QStringList names = getVisibaleNames(os);
    GT_CHECK_RESULT(names.contains(seqName), QString("sequence with name %1 not found").arg(seqName), false);

    int row = 0;
    while (names[row] != seqName) {
        row++;
    }
    QPoint center = convertCoordinates(os, QPoint(-5, row));
    QWidget* nameList = GTWidget::findWidget(os, "msa_editor_name_list");
    GT_CHECK_RESULT(nameList !=NULL, "name list is NULL", false);

    int initCoord = center.y() - getRowHeight(os)/2;
    int finalCoord = center.y() + getRowHeight(os)/2;

    for (int i = initCoord; i<finalCoord; i++){
        QPoint local = nameList->mapFromGlobal(QPoint(center.x(), i));
        QColor c = GTWidget::getColor(nameList,local);
        QString name = c.name();
        if(name == highlightningColorName){
            return true;
        }
    }

    return false;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getColor"
QString GTUtilsMSAEditorSequenceArea::getColor(U2OpStatus &os, QPoint p){
    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area", GTUtilsMdi::activeWindow(os)));
    GT_CHECK_RESULT(msaEditArea != NULL, "MsaEditorSequenceArea not found", "");

    QPoint global = convertCoordinates(os, p);
    global.setY(global.y() + (getRowHeight(os)/2 - 1));
    QPoint local = msaEditArea->mapFromGlobal(global);
    QColor c = GTWidget::getColor(msaEditArea, local);
    QString name = c.name();
    return name;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getRowHeight"
int GTUtilsMSAEditorSequenceArea::getRowHeight(U2OpStatus &os){
    QWidget* activeWindow = GTUtilsMdi::activeWindow(os);
    GT_CHECK_RESULT(activeWindow != NULL, "active mdi window is NULL", 0);
    MSAEditorUI* ui = GTUtilsMdi::activeWindow(os)->findChild<MSAEditorUI*>();
    return ui->getEditor()->getRowHeight();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "renameSequence"
void GTUtilsMSAEditorSequenceArea::renameSequence(U2OpStatus &os, const QString &seqToRename, const QString &newName){
    int num = getVisibaleNames(os).indexOf(seqToRename);
    GT_CHECK(num != -1, "sequence not found");

    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, newName, seqToRename));
    moveTo(os, QPoint(-10,num));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

void GTUtilsMSAEditorSequenceArea::createColorScheme(U2OpStatus &os, const QString &colorSchemeName, const NewColorSchemeCreator::alphabet al){
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(1, 1));
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList( ) << "Colors"
        << "Custom schemes" << "Create new color scheme" ) );
    GTUtilsDialog::waitForDialog( os, new NewColorSchemeCreator( os, colorSchemeName, al) );
    GTMouseDriver::click( os, Qt::RightButton );
}

void GTUtilsMSAEditorSequenceArea::deleteColorScheme(U2OpStatus &os, const QString &colorSchemeName){
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(1, 1));
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList( ) << "Colors"
        << "Custom schemes" << "Create new color scheme" ) );
    GTUtilsDialog::waitForDialog( os, new NewColorSchemeCreator( os, colorSchemeName, NewColorSchemeCreator::nucl,
                                                                 NewColorSchemeCreator::Delete) );
    GTMouseDriver::click( os, Qt::RightButton );
}

#undef GT_CLASS_NAME

} // namespace
