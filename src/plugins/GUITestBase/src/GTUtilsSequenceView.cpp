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

#include <QtGui/QClipboard>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QMainWindow>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMainWindow>
#endif

#include <U2Core/AnnotationSettings.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceSelection.h>

#include <U2Gui/MainWindow.h>

#include <U2View/ADVConstants.h>
#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/DetView.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/GSequenceLineViewAnnotated.h>
#include <U2View/GSequenceGraphView.h>
#include <U2View/Overview.h>

#include "utils/GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSequenceView.h"

#include "system/GTClipboard.h"
#include "GTGlobals.h"
#include <drivers/GTKeyboardDriver.h>
#include "utils/GTKeyboardUtils.h"
#include "primitives/GTMenu.h"
#include <drivers/GTMouseDriver.h>
#include <primitives/GTToolbar.h>

#include "primitives/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/RangeSelectionDialogFiller.h"

namespace U2 {
using namespace HI;

#define GT_CLASS_NAME "GTSequenceReader"
#define GT_METHOD_NAME "commonScenario"
class GTSequenceReader : public Filler {
public:
    GTSequenceReader(HI::GUITestOpStatus &_os, QString *_str):Filler(_os, "EditSequenceDialog"), str(_str){}
    void commonScenario() {
        QWidget *widget = QApplication::activeModalWidget();
        GT_CHECK(widget != NULL, "active widget not found");

        QPlainTextEdit *textEdit = widget->findChild<QPlainTextEdit*>();
        GT_CHECK(textEdit != NULL, "PlainTextEdit not found");

        *str = textEdit->toPlainText();

        QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", widget));
        GT_CHECK(box != NULL, "buttonBox is NULL");
        QPushButton* button = box->button(QDialogButtonBox::Cancel);
        GT_CHECK(button !=NULL, "cancel button is NULL");
        GTWidget::click(os, button);
    }

private:
    QString *str;
};
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsSequenceView"

#define GT_METHOD_NAME "getSequenceAsString"
void GTUtilsSequenceView::getSequenceAsString(HI::GUITestOpStatus &os, QString &sequence)
{
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GT_CHECK(mdiWindow != NULL, "MDI window == NULL");

    QWidget *mdiSequenceWidget = mdiWindow->findChild<ADVSingleSequenceWidget*>();
    GTWidget::click(os, mdiSequenceWidget);

    Runnable *filler = new selectSequenceRegionDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler);

    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);

    Runnable *chooser = new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, chooser);
    Runnable *reader = new GTSequenceReader(os, &sequence);
    GTUtilsDialog::waitForDialog(os, reader);

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSequenceAsString"
QString GTUtilsSequenceView::getSequenceAsString(HI::GUITestOpStatus &os, int number) {
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GT_CHECK_RESULT(mdiWindow != NULL, "MDI window == NULL", "");

    GTWidget::click(os, getSeqWidgetByNumber(os, number));

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os));
    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(500);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_COPY << "Copy sequence"));
    GTWidget::click(os, getSeqWidgetByNumber(os, number), Qt::RightButton);
    QString result = GTClipboard::text(os);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getBeginOfSequenceAsString"

QString GTUtilsSequenceView::getBeginOfSequenceAsString(HI::GUITestOpStatus &os, int length)
{
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GT_CHECK_RESULT(mdiWindow != NULL, "MDI window == NULL", NULL);

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    Runnable *filler = new selectSequenceRegionDialogFiller(os, length);
    GTUtilsDialog::waitForDialog(os, filler);
    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);

    GTGlobals::sleep(1000); // don't touch
    QString sequence;
    Runnable *chooser = new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, chooser);
    Runnable *reader = new GTSequenceReader(os, &sequence);
    GTUtilsDialog::waitForDialog(os, reader);

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep(1000);

    return sequence;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getEndOfSequenceAsString"
QString GTUtilsSequenceView::getEndOfSequenceAsString(HI::GUITestOpStatus &os, int length)
{
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GT_CHECK_RESULT(mdiWindow != NULL, "MDI window == NULL", NULL);

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    Runnable *filler = new selectSequenceRegionDialogFiller(os, length, false);
    GTUtilsDialog::waitForDialog(os, filler);

    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000); // don't touch

    QString sequence;
    Runnable *chooser = new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, chooser);
    Runnable *reader = new GTSequenceReader(os, &sequence);
    GTUtilsDialog::waitForDialog(os, reader);

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep(1000);

    return sequence;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getLengthOfSequence"
int GTUtilsSequenceView::getLengthOfSequence(HI::GUITestOpStatus &os)
{
    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK_RESULT(mw != NULL, "MainWindow == NULL", 0);

    MWMDIWindow *mdiWindow = mw->getMDIManager()->getActiveWindow();
    GT_CHECK_RESULT(mdiWindow != NULL, "MDI window == NULL", 0);

    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    int length = -1;
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, &length));
    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);

    return length;
}
#undef GT_METHOD_NAME

int GTUtilsSequenceView::getVisiableStart(HI::GUITestOpStatus &os, int widgetNumber){
    return getSeqWidgetByNumber(os, widgetNumber)->getDetView()->getVisibleRange().startPos;
}

#define GT_METHOD_NAME "getVisibleRange"
U2Region GTUtilsSequenceView::getVisibleRange(HI::GUITestOpStatus &os, int widgetNumber) {
    ADVSingleSequenceWidget* seqWgt = getSeqWidgetByNumber(os, widgetNumber);
    GT_CHECK_RESULT(seqWgt != NULL, "Cannot find sequence view", U2Region());
    return seqWgt->getDetView()->getVisibleRange();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkSequence"
void GTUtilsSequenceView::checkSequence(HI::GUITestOpStatus &os, const QString &expectedSequence)
{
    QString actualSequence;
    getSequenceAsString(os, actualSequence);

    GT_CHECK(expectedSequence == actualSequence, "Actual sequence does not match with expected sequence");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectSequenceRegion"
void GTUtilsSequenceView::selectSequenceRegion(HI::GUITestOpStatus &os, int from, int to)
{
    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK(mw != NULL, "MainWindow == NULL");

    MWMDIWindow *mdiWindow = mw->getMDIManager()->getActiveWindow();
    GT_CHECK(mdiWindow != NULL, "MDI window == NULL");

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, from, to));

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openSequenceView"
void GTUtilsSequenceView::openSequenceView(HI::GUITestOpStatus &os, const QString &sequenceName){
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Open View" << "action_open_view", GTGlobals::UseMouse));

    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, sequenceName);
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::click(os, Qt::RightButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addSequenceView"
void GTUtilsSequenceView::addSequenceView(HI::GUITestOpStatus &os, const QString &sequenceName){
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "submenu_add_view" << "action_add_view", GTGlobals::UseMouse));

    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, sequenceName);
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::click(os, Qt::RightButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "goToPosition"
void GTUtilsSequenceView::goToPosition(HI::GUITestOpStatus &os, int position) {
    QToolBar* toolbar = GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI);
    GT_CHECK(NULL != toolbar, "Can't find the toolbar");

    QWidget* positionLineEdit = GTWidget::findWidget(os, "go_to_pos_line_edit", toolbar);
    GT_CHECK(NULL != positionLineEdit, "Can't find the position line edit");

    GTWidget::click(os, positionLineEdit);
    GTKeyboardDriver::keySequence(os, QString::number(position));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSeqWidgetByNumber"
ADVSingleSequenceWidget* GTUtilsSequenceView::getSeqWidgetByNumber(HI::GUITestOpStatus &os, int number, const GTGlobals::FindOptions &options){
    QWidget *widget = GTWidget::findWidget(os,
        QString("ADV_single_sequence_widget_%1").arg(number),
        GTUtilsMdi::activeWindow(os), options);

    ADVSingleSequenceWidget *seqWidget = qobject_cast<ADVSingleSequenceWidget*>(widget);

    if(options.failIfNull){
        GT_CHECK_RESULT(NULL != widget, QString("Sequence widget %1 was not found!").arg(number), NULL);
    }

    return seqWidget;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPanViewByNumber"
PanView* GTUtilsSequenceView::getPanViewByNumber(HI::GUITestOpStatus &os, int number, const GTGlobals::FindOptions &options){
    ADVSingleSequenceWidget* seq = getSeqWidgetByNumber(os, number, options);
    if(options.failIfNull){
        GT_CHECK_RESULT(seq != NULL, QString("sequence view with num %1 not found").arg(number), NULL);
    }else {
        return NULL;
    }

    PanView* result = seq->findChild<PanView*>();
    if(options.failIfNull){
        GT_CHECK_RESULT(seq != NULL, QString("pan view with number %1 not fount").arg(number), NULL)
    }

    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getOverViewByNumber"
Overview* GTUtilsSequenceView::getOverviewByNumber(HI::GUITestOpStatus &os, int number, const GTGlobals::FindOptions &options){
    ADVSingleSequenceWidget* seq = getSeqWidgetByNumber(os, number, options);
    if(options.failIfNull){
        GT_CHECK_RESULT(seq != NULL, QString("sequence view with num %1 not found").arg(number), NULL);
    }else {
        return NULL;
    }

    Overview* result = seq->findChild<Overview*>();
    if(options.failIfNull){
        GT_CHECK_RESULT(seq != NULL, QString("pan view with number %1 not fount").arg(number), NULL)
    }

    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSeqWidgetsNumber"
int GTUtilsSequenceView::getSeqWidgetsNumber(HI::GUITestOpStatus &os) {
    QList<ADVSingleSequenceWidget*> seqWidgets = GTUtilsMdi::activeWindow(os)->findChildren<ADVSingleSequenceWidget*>();
    return seqWidgets.size();
}
#undef GT_METHOD_NAME

QVector<U2Region> GTUtilsSequenceView::getSelection(HI::GUITestOpStatus &os, int number){
    PanView* panView = getPanViewByNumber(os, number);
    QVector<U2Region> result = panView->getSequenceContext()->getSequenceSelection()->getSelectedRegions();
    return result;
}

#define GT_METHOD_NAME "getSeqName"
QString GTUtilsSequenceView::getSeqName(HI::GUITestOpStatus &os, int number) {
    return getSeqName(os, getSeqWidgetByNumber(os, number));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSeqName"
QString GTUtilsSequenceView::getSeqName(HI::GUITestOpStatus &os, ADVSingleSequenceWidget* seqWidget){
    GT_CHECK_RESULT(NULL != seqWidget, "Sequence widget is NULL!", "");
    QLabel *nameLabel = qobject_cast<QLabel*>(GTWidget::findWidget(os, "nameLabel", seqWidget));
    GT_CHECK_RESULT(NULL != nameLabel, "Name label is NULL!", "");

    QString labelText = nameLabel->text();
    QString result = labelText.left(labelText.indexOf("[")-1);//detachment of name from label text
    return result;
}
#undef GT_METHOD_NAME

#define MIN_ANNOTATION_WIDTH 5

#define GT_METHOD_NAME "clickAnnotation"
void GTUtilsSequenceView::clickAnnotationDet(HI::GUITestOpStatus &os, QString name, int startpos, int number, Qt::MouseButton button){
    ADVSingleSequenceWidget* seq = getSeqWidgetByNumber(os, number);
    GSequenceLineViewRenderArea* area = seq->getDetView()->getRenderArea();
    DetViewRenderArea* det = dynamic_cast<DetViewRenderArea*>(area);
    GT_CHECK(det != NULL, "det view render area not found");

    ADVSequenceObjectContext* context = seq->getSequenceContext();
    context->getAnnotationObjects(true);

    QList<Annotation*> anns;
    foreach (const AnnotationTableObject *ao, context->getAnnotationObjects(true)) {
        foreach (Annotation *a, ao->getAnnotations()) {
            if(a->getLocation().data()->regions.first().startPos == startpos-1 && a->getName() == name){
                anns<<a;
            }
        }
    }
    GT_CHECK(anns.size() != 0, QString("Annotation with name %1 and startPos %2").arg(name).arg(startpos));
    GT_CHECK(anns.size() == 1, QString("Several annotation with name %1 and startPos %2. Number is: %3").arg(name).arg(startpos).arg(anns.size()));

    Annotation* a = anns.first();
    int center = a->getLocation().data()->regions.first().center();
    goToPosition(os, center);
    GTGlobals::sleep();

    const SharedAnnotationData &aData = a->getData();
    AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry();
    AnnotationSettings* as = asr->getAnnotationSettings(aData);


    const U2Region &vr = seq->getDetView()->getVisibleRange();
    const U2Region &r = a->getLocation().data()->regions.first();

    const U2Region visibleLocation = r.intersect(vr);

    U2Region y;
    y = det->getAnnotationYRange(a, 0, as);

    float start = visibleLocation.startPos;
    float end = visibleLocation.endPos();
    if (seq->getDetView()->isWrapMode()) {
        end = visibleLocation.startPos + seq->getDetView()->getSymbolsPerLine();
    }
    float x1f = (float)(start - vr.startPos) * det->getCharWidth();
    float x2f = (float)(end - vr.startPos) * det->getCharWidth();

    int rw = qMax(MIN_ANNOTATION_WIDTH, qRound(x2f - x1f));
    int x1 = qRound(x1f);

    const QRect annotationRect(x1, y.startPos, rw, y.length);
    GTMouseDriver::moveTo(os, det->mapToGlobal(annotationRect.center()));
    GTMouseDriver::click(os, button);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getGraphView"
GSequenceGraphView *GTUtilsSequenceView::getGraphView(HI::GUITestOpStatus &os){
    GSequenceGraphView* graph = getSeqWidgetByNumber(os)->findChild<GSequenceGraphView*>();
    GT_CHECK_RESULT(graph != NULL, "Graph view is NULL", NULL);
    return graph;
}
#undef GT_METHOD_NAME

QList<QVariant> GTUtilsSequenceView::getLabelPositions(HI::GUITestOpStatus &os, GSequenceGraphView *graph){
    Q_UNUSED(os);
    QList<QVariant> list;
    graph->getLabelPositions(list);
    return list;
}

QList<TextLabel *> GTUtilsSequenceView::getGraphLabels(HI::GUITestOpStatus &os, GSequenceGraphView *graph){
    Q_UNUSED(os);
    QList<TextLabel*> result = graph->findChildren<TextLabel*>();
    return result;
}

QColor GTUtilsSequenceView::getGraphColor(HI::GUITestOpStatus &os, GSequenceGraphView *graph){
    ColorMap map = graph->getGSequenceGraphDrawer()->getColors();
    QColor result = map.value("Default color");
    return result;
}

#undef MIN_ANNOTATION_WIDTH

#undef GT_CLASS_NAME

} // namespace U2
