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
#include "DistanceMatrixDialogFiller.h"

#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTComboBox.h"
#include "api/GTCheckBox.h"
#include "api/GTGlobals.h"

#include <QtCore/QDir>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#endif
namespace U2{

#define GT_CLASS_NAME "GTUtilsDialog::DistanceMatrixDialogFiller"

DistanceMatrixDialogFiller::DistanceMatrixDialogFiller(U2OpStatus &os, bool _hamming, bool _counts, bool _excludeGaps) :
    Filler(os,"DistanceMatrixMSAProfileDialog"),
    hamming(_hamming),
    counts(_counts),
    excludeGaps(_excludeGaps),
    saveToFile(false),
    format(HTML)
{

}

DistanceMatrixDialogFiller::DistanceMatrixDialogFiller(U2OpStatus &os, DistanceMatrixDialogFiller::SaveFormat _format, QString _path) :
    Filler(os,"DistanceMatrixMSAProfileDialog"),
    hamming(true),
    counts(true),
    excludeGaps(true),
    saveToFile(true),
    format(_format),
    path(_path)
{

}

DistanceMatrixDialogFiller::DistanceMatrixDialogFiller(U2OpStatus &os, CustomScenario *c):
    Filler(os, "DistanceMatrixMSAProfileDialog", c){}

#define GT_METHOD_NAME "run"
void DistanceMatrixDialogFiller::commonScenario(){

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog!=NULL, "dialog not found");


    QComboBox* algoCombo = dialog->findChild<QComboBox*>("algoCombo");
    if(hamming){
        GTComboBox::setCurrentIndex(os,algoCombo,0);
    }
    else{
        GTComboBox::setCurrentIndex(os,algoCombo,1);
    }

    if(counts){
        QRadioButton* countsRB = dialog->findChild<QRadioButton*>("countsRB");
        GTRadioButton::click(os,countsRB);
    }
    else{
        QRadioButton* percentsRB = dialog->findChild<QRadioButton*>("percentsRB");
        GTRadioButton::click(os,percentsRB);
    }

    if(saveToFile){
        QGroupBox* saveBox = dialog->findChild<QGroupBox*>("saveBox");
        saveBox->setChecked(true);

        QLineEdit* fileEdit = dialog->findChild<QLineEdit*>("fileEdit");
        GTLineEdit::setText(os, fileEdit,path);

        if(format == HTML){
            QRadioButton* htmlRB = dialog->findChild<QRadioButton*>("htmlRB");
            GTRadioButton::click(os,htmlRB);
        }
        else{
            QRadioButton* csvRB = dialog->findChild<QRadioButton*>("csvRB");
            GTRadioButton::click(os,csvRB);
        }
    }
    QCheckBox* checkBox = dialog->findChild<QCheckBox*>("checkBox");
    GTCheckBox::setChecked(os,checkBox,excludeGaps);


    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
