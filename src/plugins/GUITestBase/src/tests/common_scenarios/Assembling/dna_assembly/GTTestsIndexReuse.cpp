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

#include <U2Gui/ToolsMenu.h>

#include "api/GTComboBox.h"
#include "api/GTMenu.h"
#include "api/GTFile.h"
#include "api/GTWidget.h"

#include "GTUtilsLog.h"
#include "GTUtilsTaskTreeView.h"

#include "runnables/qt/DefaultDialogFiller.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/corelibs/U2Gui/AlignShortReadsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ImportBAMFileDialogFiller.h"

#include "GTTestsIndexReuse.h"

namespace U2 {
namespace GUITest_index_reuse {

GUI_TEST_CLASS_DEFINITION(test_0001){
//    File will be copied to the sandbox
//    1. {Tools -> Align to reference -> Align short reads}
    GTLogTracer l("Launching Bowtie build indexer tool");
    const QString refName = "lambda_virus";
    GTFile::copy(os, testDir + "_common_data/bowtie2/lambda_virus.fa", testDir + "_common_data/scenarios/sandbox/lambda_virus.fa");

    AlignShortReadsFiller::Parameters Parameters(testDir + "_common_data/scenarios/sandbox/",
                                                               refName + ".fa",
                                                               testDir + "_common_data/bowtie2/",
                                                               "reads_1.fq",
                                                 AlignShortReadsFiller::Parameters::Bowtie);
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &Parameters));

    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os));

    QMenu* mainMenu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, mainMenu, QStringList() << ToolsMenu::NGS_MENU << ToolsMenu::NGS_MAP);
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Expected state: index is built
    GTFile::check(os, sandBoxDir + refName + ".1.ebwt");
    GTFile::check(os, sandBoxDir + refName + ".2.ebwt");
    GTFile::check(os, sandBoxDir + refName + ".3.ebwt");
    GTFile::check(os, sandBoxDir + refName + ".4.ebwt");
    GTFile::check(os, sandBoxDir + refName + ".rev.1.ebwt");
    GTFile::check(os, sandBoxDir + refName + ".rev.2.ebwt");
    GTUtilsLog::checkContainsMessage(os, l);

//    2. Repeat step 1.
    GTLogTracer l1("Launching Bowtie build indexer tool");
    GTUtilsDialog::waitForDialog(os, new DefaultDialogFiller(os, "AssemblyToRefDialog"));

    GTUtilsDialog::waitForDialog(os, new DefaultDialogFiller(os, "Import BAM File", QDialogButtonBox::Cancel));

    mainMenu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, mainMenu, QStringList() << ToolsMenu::NGS_MENU << ToolsMenu::NGS_MAP);
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Expected state: index is reused
    GTUtilsLog::checkContainsMessage(os, l1, false);

    QFileInfoList fList = QDir(sandBoxDir).entryInfoList();
    CHECK_SET_ERR(fList.size() == 14, QString("unexpected files number: %1").arg(fList.size()));

//    3. Rename index files to refName.fa.1.ebwt. Repeat step 1.
    QFile::rename(sandBoxDir + refName + ".1.ebwt", sandBoxDir + refName + ".fa.1.ebwt");
    QFile::rename(sandBoxDir + refName + ".2.ebwt", sandBoxDir + refName + ".fa.2.ebwt");
    QFile::rename(sandBoxDir + refName + ".3.ebwt", sandBoxDir + refName + ".fa.3.ebwt");
    QFile::rename(sandBoxDir + refName + ".4.ebwt", sandBoxDir + refName + ".fa.4.ebwt");
    QFile::rename(sandBoxDir + refName + ".1.ebwt", sandBoxDir + refName + ".fa.1.ebwt");
    QFile::rename(sandBoxDir + refName + ".rev.1.ebwt", sandBoxDir + refName + ".fa.rev.1.ebwt");
    QFile::rename(sandBoxDir + refName + ".rev.2.ebwt", sandBoxDir + refName + ".fa.rev.2.ebwt");

    GTLogTracer l2("Launching Bowtie build indexer tool");
    GTUtilsDialog::waitForDialog(os, new DefaultDialogFiller(os, "AssemblyToRefDialog"));

    GTUtilsDialog::waitForDialog(os, new DefaultDialogFiller(os, "Import BAM File", QDialogButtonBox::Cancel));

    mainMenu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, mainMenu, QStringList() << ToolsMenu::NGS_MENU << ToolsMenu::NGS_MAP);
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Expected state: index is reused
    GTUtilsLog::checkContainsMessage(os, l2, false);

    QFileInfoList fListRenamed = QDir(sandBoxDir).entryInfoList();
    CHECK_SET_ERR(fListRenamed.size() == 14, QString("2: unexpected files number: %1").arg(fListRenamed.size()));

//    4.  An index file is set as a reference sequence
    GTLogTracer l3("Launching Bowtie build indexer tool");
    AlignShortReadsFiller::Parameters ParametersIndex(testDir + "_common_data/scenarios/sandbox/", refName + ".fa.1.ebwt",
                                                               testDir + "_common_data/bowtie2/", "reads_1.fq", AlignShortReadsFiller::Parameters::Bowtie);
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &ParametersIndex));

    GTUtilsDialog::waitForDialog(os, new DefaultDialogFiller(os, "Import BAM File", QDialogButtonBox::Cancel));

    mainMenu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, mainMenu, QStringList() << ToolsMenu::NGS_MENU << ToolsMenu::NGS_MAP);
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Expected state: index is reused
    GTUtilsLog::checkContainsMessage(os, l3, false);

    QFileInfoList fListIndex = QDir(sandBoxDir).entryInfoList();
    CHECK_SET_ERR(fListIndex.size() == 14, QString("3: unexpected files number: %1").arg(fListIndex.size()));

}

GUI_TEST_CLASS_DEFINITION(test_0002){
    //    1. {Tools -> Align to reference -> Align short reads}
    class custom: public CustomScenario{
    public:
        void run(U2::U2OpStatus &os){
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");
            GTComboBox::setIndexWithText(os, GTWidget::findExactWidget<QComboBox *>(os, "methodNamesBox", dialog), "Bowtie");

            //    2. Set wrong file as reference
            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/CLUSTALW/COI.aln"));
            GTWidget::click(os, GTWidget::findWidget(os, "addRefButton", dialog));

            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/bowtie2/reads_1.fq"));
            GTWidget::click(os, GTWidget::findWidget(os, "addShortreadsButton", dialog));


            //    Expcted state: warning messagebox appeared
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No, "These files have the incompatible format:"));
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::QDialogButtonBox::Ok);
            GTGlobals::sleep();
            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::QDialogButtonBox::Cancel);
        }
    };
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, new custom()));

    QMenu* mainMenu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, mainMenu, QStringList() << ToolsMenu::NGS_MENU << ToolsMenu::NGS_MAP);
    GTGlobals::sleep();
}

}
}
