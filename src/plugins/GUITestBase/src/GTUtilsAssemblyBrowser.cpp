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

#include <U2View/AssemblyBrowser.h>
#include <U2View/AssemblyModel.h>

#include "GTUtilsAssemblyBrowser.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTToolbar.h"
#include "api/GTWidget.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsAnnotationsTreeView"

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

    QSharedPointer<AssemblyModel> model = assemblyBrowser->getModel();
    GT_CHECK_RESULT(!model.isNull(), "Assembly model is NULL", false);

    return model->hasReference();
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

#undef GT_CLASS_NAME

}   // namespace U2
