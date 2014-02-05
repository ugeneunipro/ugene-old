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

#include "GenerateAlignmentProfileDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTCheckBox.h"
#include "api/GTRadioButton.h"



#include <QApplication>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::GenerateAlignmentProfileDialogFiller"
#define GT_METHOD_NAME "run"
void GenerateAlignmentProfileDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    GTGlobals::sleep(500);
    if(counts){
        QRadioButton* countsRB = dialog->findChild<QRadioButton*>("countsRB");
        GTRadioButton::click(os,countsRB);
    }
    else{
        QRadioButton* percentsRB = dialog->findChild<QRadioButton*>("percentsRB");
        GTRadioButton::click(os,percentsRB);
    }

    if(!format==NONE){
        QGroupBox* saveBox = dialog->findChild<QGroupBox*>("saveBox");
        saveBox->setChecked(true);
        GTGlobals::sleep(500);

        QLineEdit* fileEdit = dialog->findChild<QLineEdit*>("fileEdit");
        GTLineEdit::setText(os,fileEdit, filePath);

        QRadioButton* formatRB = dialog->findChild<QRadioButton*>(checkBoxItems[format]);
        GTRadioButton::click(os,formatRB);
    }

    QCheckBox* gapCB = dialog->findChild<QCheckBox*>("gapCB");
    GTCheckBox::setChecked(os,gapCB,gapScore);

    QCheckBox* unusedCB = dialog->findChild<QCheckBox*>("unusedCB");
    GTCheckBox::setChecked(os,unusedCB,symdolScore);

    QCheckBox* skipGapPositionsCB = dialog->findChild<QCheckBox*>("skipGapPositionsCB");
    GTCheckBox::setChecked(os,skipGapPositionsCB,skipGaps);

    GTWidget::click(os, GTWidget::findWidget(os,"okButton"));


}
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
