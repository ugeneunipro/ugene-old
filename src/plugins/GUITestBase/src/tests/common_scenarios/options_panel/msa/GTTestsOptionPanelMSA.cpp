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
#include "api/GTComboBox.h"
#include "api/GTBaseCompleter.h"
#include "api/GTKeyboardDriver.h"

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
//    1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//    2. Open general option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::General);
//    3. Type some string which is not part some sequence's name
    QLineEdit* sequenceLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "sequenceLineEdit"));
    CHECK_SET_ERR(sequenceLineEdit != NULL, "sequenceLineEdit not found");
    GTLineEdit::setText(os, sequenceLineEdit, "wrong name");
//    Expected state: empty popup helper appeared
    bool empty = GTBaseCompleter::isEmpty(os, GTBaseCompleter::getCompleter(os));
    CHECK_SET_ERR(empty, "completer is not empty");
    GTWidget::click(os, sequenceLineEdit);//needed to close completer
}

GUI_TEST_CLASS_DEFINITION(general_test_0004){
//    1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//    2. Open general option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::General);
//    3. Rename Isophya_altaica_EF540820 to Phaneroptera_falcata
    GTUtilsMSAEditorSequenceArea::renameSequence(os, "Isophya_altaica_EF540820", "Phaneroptera_falcata");
//    4. Type "phan" in reference line edit
    QLineEdit* sequenceLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "sequenceLineEdit"));
    CHECK_SET_ERR(sequenceLineEdit != NULL, "sequenceLineEdit not found");
    GTLineEdit::setText(os, sequenceLineEdit, "Phan");
    QStringList completerList = GTBaseCompleter::getNames(os, GTBaseCompleter::getCompleter(os));
//    Expected state: two sequence names "Phaneroptera_falcata" appeared in popup helper
    CHECK_SET_ERR(completerList.count() == 2, "wrong number of sequences in completer");
    QString first = completerList.at(0);
    QString second = completerList.at(1);
    CHECK_SET_ERR(first == "Phaneroptera_falcata", QString("first sequence in completer is wrong: %1").arg(first))
    CHECK_SET_ERR(second == "Phaneroptera_falcata", QString("second sequence in completer is wrong: %1").arg(second))
    GTWidget::click(os, sequenceLineEdit);//needed to close completer
}

GUI_TEST_CLASS_DEFINITION(general_test_0005){
//    1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//    2. Open general option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::General);
//    3. Delete Hetrodes_pupus_EF540832
    GTUtilsMSAEditorSequenceArea::selectSequence(os, "Hetrodes_pupus_EF540832");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(300);
//    Expected state: Sequence number is 17
    int height = GTUtilsOptionPanelMsa::getHeight(os);
    CHECK_SET_ERR( height == 17, QString("wrong height. expected 17, found %1").arg(height));
//    4. Select one column. Press delete
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(5, 0), QPoint(5,17));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(300);
//    Expected state: Length is 603
    int length = GTUtilsOptionPanelMsa::getLength(os);
    CHECK_SET_ERR( length== 603, QString("wrong length. expected 17, found %1").arg(length));
}

GUI_TEST_CLASS_DEFINITION(highlighting_test_0001){
//    1. Open file test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Open highlighting option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Highlighting);
//    3. Select "UGENE" color scheme
    QComboBox* colorScheme = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "colorScheme"));
    GTComboBox::setIndexWithText(os, colorScheme,"UGENE");
    QString a = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,0));
    QString t = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,2));
    QString g = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(2,0));
    QString c = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,0));
    QString gap = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,2));
    CHECK_SET_ERR(a == "#fcff92", QString("a has color %1").arg(a));
    CHECK_SET_ERR(t == "#ff99b1", QString("t has color %1").arg(t));
    CHECK_SET_ERR(g == "#4eade1", QString("g has color %1").arg(g));
    CHECK_SET_ERR(c == "#70f970", QString("c has color %1").arg(c));
    CHECK_SET_ERR(gap == "#ffffff", QString("gap has color %1").arg(gap));
//    4. Check colors for all symbols
//    (branches: check Jalview, Percentage Identity, Percentage Identity(gray), UGENE color schemes)
}

GUI_TEST_CLASS_DEFINITION(highlighting_test_0001_1){
//    1. Open file test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Open highlighting option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Highlighting);
//    3. Select "No colors" color scheme
    QComboBox* colorScheme = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "colorScheme"));
    GTComboBox::setIndexWithText(os, colorScheme,"No colors");
    QString a = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,0));
    QString t = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,2));
    QString g = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(2,0));
    QString c = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,0));
    QString gap = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,2));
    CHECK_SET_ERR(a == "#ffffff", QString("a has color %1").arg(a));
    CHECK_SET_ERR(t == "#ffffff", QString("t has color %1").arg(t));
    CHECK_SET_ERR(g == "#ffffff", QString("g has color %1").arg(g));
    CHECK_SET_ERR(c == "#ffffff", QString("c has color %1").arg(c));
    CHECK_SET_ERR(gap == "#ffffff", QString("gap has color %1").arg(gap));
//    4. Check colors for all symbols
//    (branches: check Jalview, Percentage Identity, Percentage Identity(gray), UGENE color schemes)
}

GUI_TEST_CLASS_DEFINITION(highlighting_test_0001_2){
//    1. Open file test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Open highlighting option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Highlighting);
//    3. Select "Jalview" color scheme
    QComboBox* colorScheme = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "colorScheme"));
    GTComboBox::setIndexWithText(os, colorScheme,"Jalview");
    QString a = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,0));
    QString t = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,2));
    QString g = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(2,0));
    QString c = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,0));
    QString gap = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,2));
    CHECK_SET_ERR(a == "#64f73f", QString("a has color %1").arg(a));
    CHECK_SET_ERR(t == "#3c88ee", QString("t has color %1").arg(t));
    CHECK_SET_ERR(g == "#eb413c", QString("g has color %1").arg(g));
    CHECK_SET_ERR(c == "#ffb340", QString("c has color %1").arg(c));
    CHECK_SET_ERR(gap == "#ffffff", QString("gap has color %1").arg(gap));
//    4. Check colors for all symbols
//    (branches: check Jalview, Percentage Identity, Percentage Identity(gray), UGENE color schemes)
}

GUI_TEST_CLASS_DEFINITION(highlighting_test_0001_3){
//    1. Open file test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Open highlighting option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Highlighting);
//    3. Select "Percentage Identity" color scheme
    QComboBox* colorScheme = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "colorScheme"));
    GTComboBox::setIndexWithText(os, colorScheme,"Percentage Identity");
    QString a = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,0));
    QString t = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,2));
    QString g = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(2,0));
    QString c = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(5,0));
    QString gap = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,2));
    CHECK_SET_ERR(a == "#ccccff", QString("a has color %1").arg(a));
    CHECK_SET_ERR(t == "#ffffff", QString("t has color %1").arg(t));
    CHECK_SET_ERR(g == "#6464ff", QString("g has color %1").arg(g));
    CHECK_SET_ERR(c == "#9999ff", QString("c has color %1").arg(c));
    CHECK_SET_ERR(gap == "#ffffff", QString("gap has color %1").arg(gap));
//    4. Check colors for all symbols
//    (branches: check Jalview, Percentage Identity, Percentage Identity(gray), UGENE color schemes)
}

GUI_TEST_CLASS_DEFINITION(highlighting_test_0001_4){
//    1. Open file test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Open highlighting option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Highlighting);
//    3. Select "Percentage Identity (gray)" color scheme
    QComboBox* colorScheme = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "colorScheme"));
    GTComboBox::setIndexWithText(os, colorScheme,"Percentage Identity (gray)");
    QString a = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,0));
    QString t = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,2));
    QString g = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(2,0));
    QString c = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(5,0));
    QString gap = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,2));
    CHECK_SET_ERR(a == "#cccccc", QString("a has color %1").arg(a));
    CHECK_SET_ERR(t == "#ffffff", QString("t has color %1").arg(t));
    CHECK_SET_ERR(g == "#646464", QString("g has color %1").arg(g));
    CHECK_SET_ERR(c == "#999999", QString("c has color %1").arg(c));
    CHECK_SET_ERR(gap == "#ffffff", QString("gap has color %1").arg(gap));
//    4. Check colors for all symbols
//    (branches: check Jalview, Percentage Identity, Percentage Identity(gray), UGENE color schemes)
}

GUI_TEST_CLASS_DEFINITION(highlighting_test_0002){
//    1. Open file test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Create custom color scheme
    GTUtilsMSAEditorSequenceArea::createColorScheme(os, "Scheme", NewColorSchemeCreator::nucl);
//    3. Open highlighting option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Highlighting);
//    Expected state: color scheme added to "Color" combobox
    QComboBox* colorScheme = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "colorScheme"));
    GTComboBox::setIndexWithText(os, colorScheme, "No colors");
    GTComboBox::setIndexWithText(os, colorScheme, "Scheme");
//    4. Select custom scheme
//    Expected state: scheme changed
    QString a = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,0));
    QString t = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,2));
    QString g = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(2,0));
    QString c = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,0));
    QString gap = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,2));
    CHECK_SET_ERR(a == "#fcff92", QString("a has color %1").arg(a));
    CHECK_SET_ERR(t == "#ff99b1", QString("t has color %1").arg(t));
    CHECK_SET_ERR(g == "#4eade1", QString("g has color %1").arg(g));
    CHECK_SET_ERR(c == "#70f970", QString("c has color %1").arg(c));
    CHECK_SET_ERR(gap == "#ffffff", QString("gap has color %1").arg(gap));
}

GUI_TEST_CLASS_DEFINITION(highlighting_test_0002_1){
//    1. Open file test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Create 3 color schemes
    GTUtilsMSAEditorSequenceArea::createColorScheme(os, "Scheme1", NewColorSchemeCreator::nucl);
    GTUtilsMSAEditorSequenceArea::createColorScheme(os, "Scheme2", NewColorSchemeCreator::nucl);
    GTUtilsMSAEditorSequenceArea::createColorScheme(os, "Scheme3", NewColorSchemeCreator::nucl);
//    Expected state: color schemes added to "Color" combobox
     GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Highlighting);
     QComboBox* colorScheme = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "colorScheme"));
     GTComboBox::setIndexWithText(os, colorScheme, "Scheme1");
     GTComboBox::setIndexWithText(os, colorScheme, "Scheme2");
     GTComboBox::setIndexWithText(os, colorScheme, "Scheme3");
}

GUI_TEST_CLASS_DEFINITION(highlighting_test_0003){
//    1. Open file test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Create custom color scheme
    GTUtilsMSAEditorSequenceArea::createColorScheme(os, "Scheme", NewColorSchemeCreator::nucl);
//    3. Open highlighting option panel tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Highlighting);
//    4. Select custom scheme
    QComboBox* colorScheme = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "colorScheme"));
    GTComboBox::setIndexWithText(os, colorScheme, "Scheme");
//    5. Delete scheme which is selected
    GTUtilsMSAEditorSequenceArea::deleteColorScheme(os, "Scheme");
    GTGlobals::sleep(500);

//    UGENE not crashess
//    no color sheme is selected
    QString s = colorScheme->currentText();
    CHECK_SET_ERR(colorScheme->currentText() == "No colors", "wrong color scheme selected");
    QString a = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,0));
    QString t = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(0,2));
    QString g = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(2,0));
    QString c = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,0));
    QString gap = GTUtilsMSAEditorSequenceArea::getColor(os, QPoint(4,2));
    CHECK_SET_ERR(a == "#ffffff", QString("a has color %1").arg(a));
    CHECK_SET_ERR(t == "#ffffff", QString("t has color %1").arg(t));
    CHECK_SET_ERR(g == "#ffffff", QString("g has color %1").arg(g));
    CHECK_SET_ERR(c == "#ffffff", QString("c has color %1").arg(c));
    CHECK_SET_ERR(gap == "#ffffff", QString("gap has color %1").arg(gap));
}

}
}
