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

#include <QtCore/QSharedPointer>

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QLineEdit>
#else
#include <QtWidgets/QLineEdit>
#endif

#include <U2View/AssemblyBrowser.h>
#include <U2View/AssemblyModel.h>

#include "GTUtilsAssemblyBrowser.h"
#include "GTUtilsDialog.h"
#include "GTUtilsOptionsPanel.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTToolbar.h"
#include "api/GTWidget.h"
#include "runnables/qt/PopupChooser.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsAssemblyBrowser"

#define GT_METHOD_NAME "getView"
AssemblyBrowserUi *GTUtilsAssemblyBrowser::getView(U2OpStatus& os, const QString &viewTitle) {
    Q_UNUSED(os);

    const QString objectName = "assembly_browser_" + viewTitle;
    AssemblyBrowserUi* view = qobject_cast<AssemblyBrowserUi*>(GTWidget::findWidget(os, objectName));
    GT_CHECK_RESULT(NULL != view, "Assembly browser wasn't found", NULL);

    return view;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "hasReference"
bool GTUtilsAssemblyBrowser::hasReference(U2OpStatus& os, const QString &viewTitle) {
    Q_UNUSED(os);

    AssemblyBrowserUi* view = getView(os, viewTitle);
    GT_CHECK_RESULT(NULL != view, "Assembly browser wasn't found", false);

    return hasReference(os, view);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "hasReference"
bool GTUtilsAssemblyBrowser::hasReference(U2OpStatus& os, QWidget* view) {
    Q_UNUSED(os);

    AssemblyBrowserUi* assemblyBrowser = view->findChild<AssemblyBrowserUi*>("assembly_browser_" + view->objectName());
    GT_CHECK_RESULT(NULL != assemblyBrowser, "Assembly browser wasn't found", false);

    return hasReference(os, assemblyBrowser);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "hasReference"
bool GTUtilsAssemblyBrowser::hasReference(U2OpStatus &os, AssemblyBrowserUi *assemblyBrowser) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(NULL != assemblyBrowser, "Assembly browser is NULL", false);

    QSharedPointer<AssemblyModel> model = assemblyBrowser->getModel();
    GT_CHECK_RESULT(!model.isNull(), "Assembly model is NULL", false);

    return model->hasReference();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getLength"
qint64 GTUtilsAssemblyBrowser::getLength(U2OpStatus &os, const QString& viewTitle) {
    AssemblyBrowserUi* assemblyBrowser = getView(os, viewTitle);
    GT_CHECK_RESULT(NULL != assemblyBrowser, "Assembly browser wasn't found", 0);

    QWidget* optionsPanel = GTWidget::findWidget(os, "OP_MAIN_WIDGET", assemblyBrowser->parentWidget());
    GT_CHECK_RESULT(NULL != optionsPanel, "Options panel wasn't found", 0);

    QWidget* infoOptionsPanel = GTWidget::findWidget(os, "OP_OPTIONS_WIDGET", optionsPanel);
    if (!infoOptionsPanel->isVisible()) {
        GTWidget::click(os, GTWidget::findWidget(os, "OP_ASS_INFO", assemblyBrowser->parentWidget()));
        infoOptionsPanel = GTWidget::findWidget(os, "OP_OPTIONS_WIDGET", optionsPanel);
    }
    GT_CHECK_RESULT(NULL != infoOptionsPanel, "Information options panel wasn't found", 0);

    QWidget* b = GTWidget::findWidget(os, "leLength", infoOptionsPanel);
    QLineEdit* leLength = qobject_cast<QLineEdit*>(b);
    GT_CHECK_RESULT(NULL != leLength, "Length line edit wasn't found", 0);

    bool isConverted = false;
    QString lengthString = leLength->text();
    lengthString.replace(" ", "");
    qint64 value = lengthString.toLongLong(&isConverted);
    GT_CHECK_RESULT(isConverted, QString("Can't convert length to number: '%1'").arg(lengthString), 0);

    return value;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getReadsCount"
qint64 GTUtilsAssemblyBrowser::getReadsCount(U2OpStatus &os, const QString &viewTitle) {
    AssemblyBrowserUi* assemblyBrowser = getView(os, viewTitle);
    GT_CHECK_RESULT(NULL != assemblyBrowser, "Assembly browser wasn't found", 0);

    QWidget* optionsPanel = GTWidget::findWidget(os, "OP_MAIN_WIDGET", assemblyBrowser->parentWidget());
    GT_CHECK_RESULT(NULL != optionsPanel, "Options panel wasn't found", 0);

    QWidget* infoOptionsPanel = GTWidget::findWidget(os, "OP_OPTIONS_WIDGET", optionsPanel);
    if (!infoOptionsPanel->isVisible()) {
        GTWidget::click(os, GTWidget::findWidget(os, "OP_ASS_INFO", assemblyBrowser->parentWidget()));
        infoOptionsPanel = GTWidget::findWidget(os, "OP_OPTIONS_WIDGET", optionsPanel);
    }
    GT_CHECK_RESULT(NULL != infoOptionsPanel, "Information options panel wasn't found", 0);

    QLineEdit* leReads = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leReads", infoOptionsPanel));
    GT_CHECK_RESULT(NULL != leReads, "Length line edit wasn't found", 0);

    bool isConverted = false;
    QString readsString = leReads->text();
    readsString.replace(" ", "");
    qint64 value = readsString.toLongLong(&isConverted);
    GT_CHECK_RESULT(isConverted, QString("Can't convert reads count to number: '%1'").arg(readsString), 0);

    return value;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "zoomToMax"
void GTUtilsAssemblyBrowser::zoomToMax(U2OpStatus &os) {
    Q_UNUSED(os);

    QToolBar* toolbar = GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI);
    GT_CHECK(NULL != toolbar, "Can't find the toolbar");

    QWidget* zoomInButton = GTToolbar::getWidgetForActionTooltip(os, toolbar, "Zoom in");
    GT_CHECK(NULL != zoomInButton, "Can't find the 'Zoom in' button");

    while (zoomInButton->isEnabled()) {
        GTWidget::click(os, zoomInButton);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "zoomToMin"
void GTUtilsAssemblyBrowser::zoomToMin(U2OpStatus &os) {
    Q_UNUSED(os);

    QToolBar* toolbar = GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI);
    GT_CHECK(NULL != toolbar, "Can't find the toolbar");

    QWidget* zoomOutButton = GTToolbar::getWidgetForActionTooltip(os, toolbar, "Zoom out");
    GT_CHECK(NULL != zoomOutButton, "Can't find the 'Zoom in' button");

    while (zoomOutButton->isEnabled()) {
        GTWidget::click(os, zoomOutButton);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "goToPosition"
void GTUtilsAssemblyBrowser::goToPosition(U2OpStatus &os, qint64 position) {
    Q_UNUSED(os);

    QToolBar* toolbar = GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI);
    GT_CHECK(NULL != toolbar, "Can't find the toolbar");

    QWidget* positionLineEdit = GTWidget::findWidget(os, "go_to_pos_line_edit", toolbar);
    GT_CHECK(NULL != positionLineEdit, "Can't find the position line edit");

    GTWidget::click(os, positionLineEdit);
    GTKeyboardDriver::keySequence(os, QString::number(position));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "callExportCoverageDialog"
void GTUtilsAssemblyBrowser::callExportCoverageDialog(U2OpStatus &os, Area area) {
    Q_UNUSED(os);

    switch (area) {
    case Consensus:
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Export coverage"));
        GTWidget::click(os, GTWidget::findWidget(os, "Consensus area"), Qt::RightButton);
        break;
    case Overview:
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Export coverage"));
        GTWidget::click(os, GTWidget::findWidget(os, "Zoomable assembly overview"), Qt::RightButton);
        break;
    case Reads:
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Export" << "Export coverage"));
        GTWidget::click(os, GTWidget::findWidget(os, "assembly_reads_area"), Qt::RightButton);
        break;
    default:
        os.setError("Can't call the dialog on this area");
        FAIL(false, );
    }
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
