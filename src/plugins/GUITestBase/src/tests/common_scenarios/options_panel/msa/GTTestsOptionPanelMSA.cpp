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

#include "GTTestsOptionPanelMSA.h"

#include "api/GTFileDialog.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTBaseCompleter.h"

#include "GTUtilsOptionPanelMSA.h"
#include "GTUtilsMsaEditorSequenceArea.h"
namespace U2{

namespace GUITest_common_scenarios_options_panel_MSA{

GUI_TEST_CLASS_DEFINITION(general_test_0001){
    const QString seqName = "Phaneroptera_falcata";
//    1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//    2. Open general option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::General);
//    3. Use button to add Phaneroptera_falcata as referene
    GTUtilsOptionPanelMsa::addReference(os, seqName);
//    Expected state:
//    reference sequence line edit is empty
    QLineEdit* sequenceLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "sequenceLineEdit"));
    CHECK_SET_ERR(sequenceLineEdit != NULL, "sequenceLineEdit not found");
    QString text = sequenceLineEdit->text();
    CHECK_SET_ERR(text == seqName, QString("sequenceLineEdit contains %1, expected: %2").arg(text).arg(seqName));
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::isSequenceHightighted(os, seqName), "sequence not highlighted");
//    Expected state: Phaneroptera_falcata highlighted as reference

//    4. Use button to remove reference
    GTUtilsOptionPanelMsa::removeReference(os);
//    Expected state:
//    reference sequence line edit contains "select and add"
//    Phaneroptera_falcata is not highlighted as reference
    text = sequenceLineEdit->text();
    CHECK_SET_ERR(text.isEmpty(), QString("sequenceLineEdit contains %1, no text ecpected").arg(text));
    CHECK_SET_ERR(!GTUtilsMSAEditorSequenceArea::isSequenceHightighted(os, seqName), "sequence not highlighted");
}

GUI_TEST_CLASS_DEFINITION(general_test_0001_1){
    //Difference: pupup completer is used
    const QString seqName = "Phaneroptera_falcata";
//    1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//    2. Open general option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::General);
//    3. Use button to add Phaneroptera_falcata as reference
    GTUtilsOptionPanelMsa::addReference(os, seqName, GTUtilsOptionPanelMsa::Completer);
//    Expected state:
//    reference sequence line edit contains Phaneroptera_falcata
    QLineEdit* sequenceLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "sequenceLineEdit"));
    CHECK_SET_ERR(sequenceLineEdit != NULL, "sequenceLineEdit not found");
    QString text = sequenceLineEdit->text();
    CHECK_SET_ERR(text == seqName, QString("sequenceLineEdit contains %1, expected: %2").arg(text).arg(seqName));
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::isSequenceHightighted(os, seqName), "sequence not highlighted");
//    Phaneroptera_falcata highlighted as reference

//    4. Use button to remove reference
    GTUtilsOptionPanelMsa::removeReference(os);
//    Expected state:
//    reference sequence line edit contains "select and add"
//    Phaneroptera_falcata is not highlighted as reference
    text = sequenceLineEdit->text();
    CHECK_SET_ERR(text.isEmpty(), QString("sequenceLineEdit contains %1, no text ecpected").arg(text));
    CHECK_SET_ERR(!GTUtilsMSAEditorSequenceArea::isSequenceHightighted(os, seqName), "sequence not highlighted");
}

GUI_TEST_CLASS_DEFINITION(general_test_0002){
    const QString seqName = "Phaneroptera_falcata";
//1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//2. Open general option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::General);
//3. Type "phan" in reference line edit
    QLineEdit* sequenceLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "sequenceLineEdit"));
    CHECK_SET_ERR(sequenceLineEdit != NULL, "sequenceLineEdit not found");
    GTLineEdit::setText(os, sequenceLineEdit, "phan");
    QStringList names = GTBaseCompleter::getNames(os, GTBaseCompleter::getCompleter(os));
//Expected state: popup helper contains Phaneroptera_falcata.(case insencivity is checked)
    int num = names.count();
    CHECK_SET_ERR(num == 1, QString("wrong number of sequences in completer. Expected 1, found %1").arg(num));

    QString name = names.at(0);
    CHECK_SET_ERR(name == seqName, QString("wrong sequence name. Expected %1, found %2").arg(seqName).arg(name));
}

GUI_TEST_CLASS_DEFINITION(general_test_0003){

}
}
}
