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


#include "GTUtilsOptionPanelMSA.h"
#include "api/GTWidget.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTTreeWidget.h"
#include "api/GTBaseCompleter.h"

#include "GTUtilsMsaEditorSequenceArea.h"

#include <QtGui/QTreeWidget>


namespace U2{
QMap<GTUtilsOptionPanelMsa::Tabs, QString> GTUtilsOptionPanelMsa::initNames(){
    QMap<Tabs,QString> result;
    result.insert(General, "OP_MSA_GENERAL");
    result.insert(Highlighting, "OP_MSA_HIGHLIGHTING");
    result.insert(PairwiseAlignment, "OP_PAIRALIGN");
    result.insert(TreeSettings, "OP_MSA_ADD_TREE_WIDGET");
    result.insert(ExportConsensus, "OP_EXPORT_CONSENSUS");
    result.insert(Statistics, "OP_SEQ_STATISTICS_WIDGET");
    return result;
}
const QMap<GTUtilsOptionPanelMsa::Tabs,QString> GTUtilsOptionPanelMsa::tabsNames = initNames();

#define GT_CLASS_NAME "GTUtilsOptionPanelMSA"

#define GT_METHOD_NAME "openTab"
void GTUtilsOptionPanelMsa::openTab(U2OpStatus &os, Tabs tab){
    GTWidget::click(os, GTWidget::findWidget(os, tabsNames[tab]));
    GTGlobals::sleep(200);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addReference"
void GTUtilsOptionPanelMsa::addReference(U2OpStatus &os, QString seqName, AddRefMethod method){
    GT_CHECK(!seqName.isEmpty(), "sequence name is empty");
    //Option panel should be opned to use this method
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);

    GT_CHECK(nameList.contains(seqName), QString("sequence with name %1 not found").arg(seqName));

    switch (method)
    {
    case Button:
        GTUtilsMSAEditorSequenceArea::selectSequence(os, seqName);
        GTWidget::click(os, GTWidget::findWidget(os, "addSeq"));
        break;
    case Completer:
        QWidget* sequenceLineEdit = GTWidget::findWidget(os, "sequenceLineEdit");
        GTWidget::click(os, sequenceLineEdit);
        GTKeyboardDriver::keyClick(os, seqName.at(0).toAscii());
        GTGlobals::sleep(200);
        QTreeWidget* completer = sequenceLineEdit->findChild<QTreeWidget*>();
        GT_CHECK(completer != NULL, "auto completer widget not found");
        GTBaseCompleter::click(os, completer, seqName);
        break;
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "removeReference"
void GTUtilsOptionPanelMsa::removeReference(U2OpStatus &os){
    GTWidget::click(os, GTWidget::findWidget(os, "deleteSeq"));
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME
}
