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

#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>

#include <U2Core/U2ObjectDbi.h>

#include "GTUtilsTaskTreeView.h"
#include "ImportPrimersDialogFiller.h"
#include "api/GTComboBox.h"
#include "api/GTFileDialog.h"
#include "api/GTWidget.h"
#include "runnables/ugene/corelibs/U2Gui/ProjectTreeItemSelectorDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/SharedConnectionsDialogFiller.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ImportPrimersDialogFiller"

ImportPrimersDialogFiller::ImportPrimersDialogFiller(U2OpStatus &os, const QStringList &fileList) :
    Filler(os, "ImportPrimersDialog"),
    fileList(fileList)
{

}

ImportPrimersDialogFiller::ImportPrimersDialogFiller(U2OpStatus &os, CustomScenario *scenario) :
    Filler(os, "ImportPrimersDialog", scenario)
{

}

#define GT_METHOD_NAME "commonScenario"
void ImportPrimersDialogFiller::commonScenario() {
    CHECK_EXT(!fileList.isEmpty(), GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Cancel), );

    foreach (const QString &file, fileList) {
        addFile(os, file);
    }

    GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setImportTarget"
void ImportPrimersDialogFiller::setImportTarget(U2OpStatus &os, ImportSource importSource) {
    switch (importSource) {
    case LocalFiles:
        GTComboBox::setIndexWithText(os, GTWidget::findExactWidget<QComboBox *>(os, "cbSource", getDialog(os)), "Local file(s)");
        break;
    case SharedDb:
        GTComboBox::setIndexWithText(os, GTWidget::findExactWidget<QComboBox *>(os, "cbSource", getDialog(os)), "Shared database");
        break;
    default:
        os.setError("Unexpected import source");
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addFile"
void ImportPrimersDialogFiller::addFile(U2OpStatus &os, const QString &filePath) {
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, filePath));
    GTWidget::click(os, GTWidget::findWidget(os, "pbAddFile", getDialog(os)));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "connectDatabase"
void ImportPrimersDialogFiller::connectDatabase(U2OpStatus &os, const QString &databaseName) {
    QList<SharedConnectionsDialogFiller::Action> actions;
    actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::CLICK, databaseName);
    actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::CONNECT);
    GTUtilsDialog::waitForDialog(os, new SharedConnectionsDialogFiller(os, actions, SharedConnectionsDialogFiller::UNSAFE));
    GTWidget::click(os, GTWidget::findWidget(os, "pbConnect", getDialog(os)));
    GTUtilsTaskTreeView::waitTaskFinished(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addObject"
void ImportPrimersDialogFiller::addObjects(U2OpStatus &os, const QString &databaseName, const QStringList &objectNames) {
    QMap<QString, QStringList> objects;
    objects[databaseName] = objectNames;
    GTUtilsDialog::waitForDialog(os, new ProjectTreeItemSelectorDialogFiller(os,
                                                                             objects,
                                                                             QSet<GObjectType>() << GObjectTypes::SEQUENCE,
                                                                             ProjectTreeItemSelectorDialogFiller::Separate));
    GTWidget::click(os, GTWidget::findWidget(os, "pbAddObject", getDialog(os)));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addObject"
void ImportPrimersDialogFiller::addObjects(U2OpStatus &os, const QMap<QString, QStringList> &databaseAndObjectNames) {
    GTUtilsDialog::waitForDialog(os, new ProjectTreeItemSelectorDialogFiller(os,
                                                                             databaseAndObjectNames,
                                                                             QSet<GObjectType>() << GObjectTypes::SEQUENCE,
                                                                             ProjectTreeItemSelectorDialogFiller::Separate));
    GTWidget::click(os, GTWidget::findWidget(os, "pbAddObject", getDialog(os)));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getDialog"
QWidget *ImportPrimersDialogFiller::getDialog(U2OpStatus &os) {
    Q_UNUSED(os);
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK_RESULT(NULL != dialog, "Active modal dialog is NULL", NULL);
    return dialog;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
