/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "GTUtilsProject.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTSequenceReadingModeDialogUtils.h"
#include "api/GTMenu.h"
#include "api/GTLineEdit.h"
#include "api/GTWidget.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsProjectTreeView.h"

#include "GTUtilsAnnotationsTreeView.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2View/ADVConstants.h>
#include <U2Gui/ObjectViewModel.h>
#include <QtGui/QMainWindow>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QHeaderView>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsProject"

void GTUtilsProject::openFiles(U2OpStatus &os, const QList<QUrl> &urls, const OpenFileSettings& s) {

    switch (s.openMethod) {
        case OpenFileSettings::DragDrop:
        default:
            openFilesDrop(os, urls);
    }

    checkProject(os);
}

void GTUtilsProject::openFiles(U2OpStatus &os, const GUrl &path, const OpenFileSettings& s) {
    openFiles(os, QList<QUrl>() << path.getURLString(), s);
}

#define GT_METHOD_NAME "checkProject"
void GTUtilsProject::checkProject(U2OpStatus &os, CheckType checkType) {

    GTGlobals::sleep(500);

    if (checkType == NotExists) {
        GT_CHECK(AppContext::getProject() == NULL, "There is a project");
        return;
    }

    GT_CHECK(AppContext::getProject() != NULL, "There is no project");
    if (checkType == Empty) {
        GT_CHECK(AppContext::getProject()->getDocuments().isEmpty() == true, "Project is not empty");
    }
}
#undef GT_METHOD_NAME

void GTUtilsProject::openFilesDrop(U2OpStatus &os, const QList<QUrl>& urls) {

    QWidget* widget = AppContext::getMainWindow()->getQMainWindow();
    QPoint widgetPos(widget->width()/2, widget->height()/2);

    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls(urls);

    Qt::DropActions dropActions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    Qt::MouseButtons mouseButtons = Qt::LeftButton;

    if (urls.size() > 1) {
        GTUtilsDialog::waitForDialog(os, new GTSequenceReadingModeDialogUtils(os));
    }

    QDragEnterEvent* dragEnterEvent = new QDragEnterEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
    GTGlobals::sendEvent(widget, dragEnterEvent);

    QDropEvent* dropEvent = new QDropEvent(widgetPos, dropActions, mimeData, mouseButtons, 0);
    GTGlobals::sendEvent(widget, dropEvent);
}

#undef GT_CLASS_NAME

} // U2
