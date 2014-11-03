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

#include <QApplication>

#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsPcr.h"
#include "GTUtilsPrimerLibrary.h"
#include "api/GTLineEdit.h"
#include "api/GTMouseDriver.h"
#include "runnables/ugene/plugins/pcr/AddPrimerDialogFiller.h"
#include "runnables/ugene/plugins/pcr/PrimerLibrarySelectorFiller.h"

#include "GTTestsPrimerLibrary.h"

namespace U2 {
namespace GUITest_common_scenarios_primer_library {

GUI_TEST_CLASS_DEFINITION(test_0001) {
    //The library is the singleton MDI window

    //1. Click the menu Tools -> Primer -> Primer Library.
    //Expected: the library MDI window is opened.
    QWidget *libraryMdi1 = GTUtilsPrimerLibrary::openLibrary(os);

    //2. Click the menu again.
    QWidget *libraryMdi2 = GTUtilsPrimerLibrary::openLibrary(os);

    //Expected: the same MDI windows is opened (not the second one).
    CHECK_SET_ERR(libraryMdi1 == libraryMdi2, "Different MDI windows");

    //3. Click the close button.
    GTUtilsPrimerLibrary::clickButton(os, GTUtilsPrimerLibrary::Close);

    //Expected: The window is closed.
    QWidget *libraryMdi3 = GTUtilsMdi::activeWindow(os, GTGlobals::FindOptions(false));
    CHECK_SET_ERR(NULL == libraryMdi3, "Library MDI is not closed");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    //Add new primer:
    //    Primer line edit ACGT content
    //    Availability of the OK button

    //1. Click the menu Tools -> Primer -> Primer Library.
    GTUtilsPrimerLibrary::openLibrary(os);
    int librarySize = GTUtilsPrimerLibrary::librarySize(os);

    //2. Click the new primer button.
    //Expected: the dialog appears. The OK button is disabled.
    class Scenario : public CustomScenario {
    public:
        void run(U2OpStatus &os) {
            //3. Set the focus at the primer line edit and write "Q%1" (not ACGT).
            QLineEdit *primerEdit = dynamic_cast<QLineEdit*>(GTWidget::findWidget(os, "primerEdit"));
            GTLineEdit::setText(os, primerEdit, "Q%1", true);

            //Expected: the line edit is empty.
            CHECK_SET_ERR(primerEdit->text().isEmpty(), "Wrong input");

            //4. Write "atcg".
            GTLineEdit::setText(os, primerEdit, "atcg", true);

            //Expected: the line edit content is "ATCG". The OK button is enabled.
            CHECK_SET_ERR(primerEdit->text() == "ATCG", "No upper-case");

            //5. Remove the primer name.
            QLineEdit *nameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameEdit"));
            GTLineEdit::setText(os, nameEdit, "");

            //Expected: The OK button is disabled.
            QWidget *dialog = QApplication::activeModalWidget();
            QPushButton *okButton = GTUtilsDialog::buttonBox(os, dialog)->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR(!okButton->isEnabled(), "The OK button is enabled");

            //6. Set the name "Primer".
            GTLineEdit::setText(os, nameEdit, "Primer");

            //7. Click the OK button.
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };
    AddPrimerDialogFiller::Parameters parameters;
    parameters.scenario = new Scenario();
    GTUtilsDialog::waitForDialog(os, new AddPrimerDialogFiller(os, parameters));
    GTUtilsPrimerLibrary::clickButton(os, GTUtilsPrimerLibrary::Add);

    //Expected: the new primer appears in the table.
    CHECK_SET_ERR(librarySize + 1 == GTUtilsPrimerLibrary::librarySize(os), "Wrong primers count");
    CHECK_SET_ERR(GTUtilsPrimerLibrary::getPrimerSequence(os, librarySize) == "ATCG", "Wrong primer");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    //Remove primers:
    //    Availability of the button

    //1. Click the menu Tools -> Primer -> Primer Library.
    GTUtilsPrimerLibrary::openLibrary(os);
    int librarySize = GTUtilsPrimerLibrary::librarySize(os);

    //2. Add a new primer if the library is empty.
    AddPrimerDialogFiller::Parameters parameters;
    parameters.primer = "AAAAAAAAAAAAAA";
    GTUtilsDialog::waitForDialog(os, new AddPrimerDialogFiller(os, parameters));
    GTUtilsPrimerLibrary::clickButton(os, GTUtilsPrimerLibrary::Add);

    //3. Click the empty place of the table.
    QPoint emptyPoint = GTUtilsPrimerLibrary::getPrimerPoint(os, librarySize);
    emptyPoint.setY(emptyPoint.y() + 40);
    GTMouseDriver::moveTo(os, emptyPoint);
    GTMouseDriver::click(os);

    //Expected: The remove button is disabled.
    QAbstractButton *removeButton = GTUtilsPrimerLibrary::getButton(os, GTUtilsPrimerLibrary::Remove);
    CHECK_SET_ERR(!removeButton->isEnabled(), "The remove button is enabled");

    //4. Select the primer.
    GTMouseDriver::moveTo(os, GTUtilsPrimerLibrary::getPrimerPoint(os, librarySize));
    GTMouseDriver::click(os);

    //Expected: The remove button is enabled.
    CHECK_SET_ERR(removeButton->isEnabled(), "The remove button is disabled");

    //5. Click the button.
    GTUtilsPrimerLibrary::clickButton(os, GTUtilsPrimerLibrary::Remove);

    //Expected: the primer is disappeared from the table.
    CHECK_SET_ERR(librarySize == GTUtilsPrimerLibrary::librarySize(os), "Wrong primers count");
}

GUI_TEST_CLASS_DEFINITION(test_0004) {
    //In silico PCR with the library data:
    //    Add a primer from the library
    //    Double click

    { // Pre-test
        GTUtilsPrimerLibrary::openLibrary(os);
        AddPrimerDialogFiller::Parameters parameters;
        parameters.primer = "AAAAAAAAAAAAAA";
        GTUtilsDialog::waitForDialog(os, new AddPrimerDialogFiller(os, parameters));
        GTUtilsPrimerLibrary::clickButton(os, GTUtilsPrimerLibrary::Add);
        GTUtilsPrimerLibrary::clickButton(os, GTUtilsPrimerLibrary::Close);
    }

    //1. Open "_common_data/fasta/pcr_test.fa".
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "pcr_test.fa");

    //2. Open the PCR OP.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_IN_SILICO_PCR"));

    //3. Click the browse library button for the forward primer.
    //Expected: the library dialog appears, the OK button is disabled.
    //4. Click a primer in the table.
    //Expected: the OK button is enabled.
    //5. Double click the primer.
    GTUtilsDialog::waitForDialog(os, new PrimerLibrarySelectorFiller(os, 0, true));
    GTWidget::click(os, GTUtilsPcr::browseButton(os, U2Strand::Direct));

    //Expected: the dialog is closed, the chosen primer sequence is in the forward primer line edit.
    QLineEdit *primerEdit = GTWidget::findExactWidget<QLineEdit*>(os, "primerEdit", GTUtilsPcr::primerBox(os, U2Strand::Direct));
    CHECK_SET_ERR(primerEdit->text() == "AAAAAAAAAAAAAA", "Wrong primer");
}

} // GUITest_common_scenarios_primer_library
} // U2
