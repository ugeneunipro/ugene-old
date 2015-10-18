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

#include "GTTestsProjectUserLocking.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTFileDialog.h"
#include "api/GTWidget.h"
#include "api/GTTreeWidget.h"
#include "api/GTFile.h"
#include "api/GTMenu.h"
#include "GTUtilsProject.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/ugeneui/CreateNewProjectWidgetFiller.h"

#include <U2Core/DocumentModel.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QDialogButtonBox>
#endif

namespace U2 {

namespace GUITest_common_scenarios_project_user_locking {

GUI_TEST_CLASS_DEFINITION(test_0001) {

#define GT_CLASS_NAME "GUITest_common_scenarios_project_user_locking_test_0002::CreateAnnnotationDialogComboBoxChecker"
#define GT_METHOD_NAME "run"
    class CreateAnnnotationDialogComboBoxChecker : public Filler {
    public:
        CreateAnnnotationDialogComboBoxChecker(U2OpStatus &_os, const QString &radioButtonName): Filler(_os, "CreateAnnotationDialog"), buttonName(radioButtonName){}
        void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            GT_CHECK(dialog != NULL, "activeModalWidget is NULL");

            QRadioButton *btn = dialog->findChild<QRadioButton*>("rbExistingTable");
            GT_CHECK(btn != NULL, "Radio button not found");

            if (! btn->isEnabled()) {
                GTMouseDriver::moveTo(os, btn->mapToGlobal(btn->rect().topLeft()));
                GTMouseDriver::click(os);
            }

            QComboBox *comboBox = dialog->findChild<QComboBox*>();
            GT_CHECK(comboBox != NULL, "ComboBox not found");

            GT_CHECK(comboBox->count() == 0, "ComboBox is not empty");

            QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
            GT_CHECK(box != NULL, "buttonBox is NULL");
            QPushButton* button = box->button(QDialogButtonBox::Cancel);
            GT_CHECK(button !=NULL, "cancel button is NULL");
            GTWidget::click(os, button);
        }

    private:
        QString buttonName;
    };
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/project/", "proj5.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTGlobals::sleep(200);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(200);
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

    GTGlobals::sleep(2000);

    GTUtilsDialog::waitForDialog(os, new CreateAnnnotationDialogComboBoxChecker(os, ""));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
}

GUI_TEST_CLASS_DEFINITION(test_0002)
{
#define GT_CLASS_NAME "GUITest_common_scenarios_project_user_locking_test_0002::CreateAnnnotationDialogComboBoxChecker"
#define GT_METHOD_NAME "run"
    class CreateAnnnotationDialogComboBoxChecker : public Filler {
    public:
        CreateAnnnotationDialogComboBoxChecker(U2OpStatus &_os, const QString &radioButtonName): Filler(_os, "CreateAnnotationDialog"), buttonName(radioButtonName){}
        void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            GT_CHECK(dialog != NULL, "activeModalWidget is NULL");

            QRadioButton *btn = dialog->findChild<QRadioButton*>("rbExistingTable");
            GT_CHECK(btn != NULL, "Radio button not found");

            if (! btn->isEnabled()) {
                GTMouseDriver::moveTo(os, btn->mapToGlobal(btn->rect().topLeft()));
                GTMouseDriver::click(os);
            }

            QComboBox *comboBox = dialog->findChild<QComboBox*>();
            GT_CHECK(comboBox != NULL, "ComboBox not found");

            GT_CHECK(comboBox->count() != 0, "ComboBox is empty");

            QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
            GT_CHECK(box != NULL, "buttonBox is NULL");
            QPushButton* button = box->button(QDialogButtonBox::Cancel);
            GT_CHECK(button !=NULL, "cancel button is NULL");
            GTWidget::click(os, button);
        }

    private:
        QString buttonName;
    };
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

    // backup proj3 first
//     GTFile::backup(os, testDir + "_common_data/scenarios/project/proj3.uprj");

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj3.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");

    QModelIndex item = GTUtilsProjectTreeView::findIndex(os, "1.gb");

    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "1.gb");
    GTGlobals::sleep(100);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Open View" << "action_open_view"));
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);
    QIcon itemIconBefore = qvariant_cast<QIcon>(item.data(Qt::DecorationRole));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EDIT_MENU<<ACTION_DOCUMENT__UNLOCK));
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::click(os, Qt::RightButton);

    QIcon itemIconAfter = qvariant_cast<QIcon>(item.data(Qt::DecorationRole));
    if (itemIconBefore.cacheKey() == itemIconAfter.cacheKey() && !os.hasError()) {
        os.setError("Lock icon has not disappear");
    }

    GTUtilsDialog::waitForDialog(os, new CreateAnnnotationDialogComboBoxChecker(os, ""));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EDIT_MENU<<ACTION_DOCUMENT__LOCK));
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::click(os, Qt::RightButton);

    GTMenu::clickMainMenuItem(os, QStringList() << "File" << "Close project");
    GTGlobals::sleep();

    // proj3 was modified, restoring
//     GTFile::restore(os, testDir + "_common_data/scenarios/project/proj3.uprj");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    QIcon roDocumentIcon(":/core/images/ro_document.png");
    QIcon documentIcon(":/core/images/document.png");

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

    QModelIndex item = GTUtilsProjectTreeView::findIndex(os, "1.gb");
    QIcon icon = GTUtilsProjectTreeView::getIcon(os, item);


    QImage foundImage = icon.pixmap(32,32).toImage();
    QImage expectedImage = documentIcon.pixmap(32,32).toImage();
    CHECK_SET_ERR(expectedImage == foundImage, "Icon is locked");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< ACTION_PROJECT__EDIT_MENU << ACTION_DOCUMENT__LOCK));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1.gb"));
    GTMouseDriver::click(os, Qt::RightButton);

    icon = GTUtilsProjectTreeView::getIcon(os, item);
    foundImage = icon.pixmap(32,32).toImage();
    expectedImage = roDocumentIcon.pixmap(32,32).toImage();
    CHECK_SET_ERR(expectedImage == foundImage, "Icon is unlocked");

    GTUtilsDialog::waitForDialog(os, new SaveProjectAsDialogFiller(os, "proj2", testDir+"_common_data/scenarios/sandbox", "proj2"));
    GTMenu::clickMainMenuItem(os, QStringList() << "File" << "Save project as...");
    GTGlobals::sleep();

    GTMenu::clickMainMenuItem(os, QStringList() << "File" << "Close project");
    GTGlobals::sleep();

    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/sandbox/proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");

    item = GTUtilsProjectTreeView::findIndex(os, "1.gb");
    icon = GTUtilsProjectTreeView::getIcon(os, item);
    foundImage = icon.pixmap(32,32).toImage();
    expectedImage = roDocumentIcon.pixmap(32,32).toImage();
    CHECK_SET_ERR(expectedImage == foundImage, "Icon is unlocked");
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
    GTUtilsProject::openFiles(os, dataDir + "samples/ABIF/A01.abi");
    GTUtilsProject::openFiles(os, dataDir + "samples/Genbank/sars.gb");
    Document* d = GTUtilsDocument::getDocument(os,"A01.abi");
    CHECK_SET_ERR(!d->isModificationAllowed(StateLockModType_AddChild), QString("Enable to perform locking/unlocking for : %1").arg(d->getName()));

    d = GTUtilsDocument::getDocument(os,"sars.gb");
    CHECK_SET_ERR(d->isModificationAllowed(StateLockModType_AddChild), QString("Enable to perform locking/unlocking for : %1").arg(d->getName()));
}
} // GUITest_common_scenarios_project_user_locking namespace

} // U2 namespace
