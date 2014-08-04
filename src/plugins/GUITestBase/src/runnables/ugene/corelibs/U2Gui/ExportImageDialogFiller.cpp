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

#include "ExportImageDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTRadioButton.h"
#include "api/GTCheckBox.h"
#include "api/GTMouseDriver.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#endif
#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
#include <QtGui/QTableWidget>


namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ExportImage"
#define GT_METHOD_NAME "run"
void ExportImage::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit* fileEdit = dialog->findChild<QLineEdit*>("fileNameEdit");
    GTLineEdit::setText(os,fileEdit, filePath);

    if(comboValue != ""){
        QComboBox* formatsBox = dialog->findChild<QComboBox*>("formatsBox");
        GTComboBox::setIndexWithText(os, formatsBox,comboValue);
    }

    if(spinValue){
        QSpinBox* spin = dialog->findChild<QSpinBox*>("qualitySpinBox");
        GTSpinBox::setValue(os, spin, spinValue, GTGlobals::UseKeyBoard);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "cancel button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ExportMsaImage"
#define GT_METHOD_NAME "run"
void ExportMsaImage::run() {
    GT_CHECK( (exportWholeAlignment && exportCurrentSelection) != true, "Wrong filler parameters");

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (!exportWholeAlignment) {
        if (!exportCurrentSelection) {
            GTUtilsDialog::waitForDialog(os, new SelectSubalignmentFiller(os, region));
        }
        QComboBox* exportType = dialog->findChild<QComboBox*>("comboBox");
        GTComboBox::setIndexWithText(os, exportType, "Custom region");
    }

    QCheckBox* namesCB = dialog->findChild<QCheckBox*>("exportSeqNames");
    GTCheckBox::setChecked(os, namesCB, settings.includeNames);

    QCheckBox* consensusCB = dialog->findChild<QCheckBox*>("exportConsensus");
    GTCheckBox::setChecked(os, consensusCB, settings.includeConsensus);

    QCheckBox* rulerCB= dialog->findChild<QCheckBox*>("exportRuler");
    GTCheckBox::setChecked(os, rulerCB, settings.includeRuler);

    QLineEdit* fileEdit = dialog->findChild<QLineEdit*>("fileNameEdit");
    GTLineEdit::setText(os,fileEdit, filePath);

    if(comboValue != ""){
        QComboBox* formatsBox = dialog->findChild<QComboBox*>("formatsBox");
        GTComboBox::setIndexWithText(os, formatsBox,comboValue);
    }

    if(spinValue){
        QSpinBox* spin = dialog->findChild<QSpinBox*>("qualitySpinBox");
        GTSpinBox::setValue(os, spin, spinValue, GTGlobals::UseKeyBoard);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::SelectSubalignmentFiller"
#define GT_METHOD_NAME "run"
void SelectSubalignmentFiller::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QSpinBox* startPosBox = dialog->findChild<QSpinBox*>("startPosBox");
    GT_CHECK(startPosBox != NULL, "startPosBox is NULL");
    GTSpinBox::setValue(os, startPosBox, msaRegion.region.startPos);

    QSpinBox* endPosBox = dialog->findChild<QSpinBox*>("endPosBox");
    GT_CHECK(endPosBox != NULL, "endPoxBox is NULL");
    GTSpinBox::setValue(os, endPosBox, msaRegion.region.endPos());

    QWidget *noneButton = dialog->findChild<QWidget*>("noneButton");
    GT_CHECK(noneButton!=NULL, "noneButton is NULL");
    GTWidget::click(os, noneButton);

    QTableWidget *table = dialog->findChild<QTableWidget*>("sequencesTableWidget");
    GT_CHECK(table!=NULL, "tableWidget is NULL");

    QPoint p = table->geometry().topRight();
    p.setX(p.x() - 2);
    p.setY(p.y() + 2);
    p = dialog->mapToGlobal(p);

    GTMouseDriver::moveTo(os,p);
    GTMouseDriver::click(os);
    for(int i = 0; i < table->rowCount();i++){
        foreach (QString s, msaRegion.sequences){
            QCheckBox *box = qobject_cast<QCheckBox*>(table->cellWidget(i,0));
            if (s == box->text()){
                GT_CHECK(box->isEnabled(), QString("%1 box is disabled").arg(box->text()));
                box->setChecked(true);
            }
        }
    }

    QDialogButtonBox* box = dialog->findChild<QDialogButtonBox*>("buttonBox");
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* ok = box->button(QDialogButtonBox::Ok);
    GT_CHECK(ok !=NULL, "ok button is NULL");
    GTWidget::click(os, ok);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

ImageExportFormFiller::ImageExportFormFiller(U2OpStatus &os, const Parameters &parameters)
: Filler(os, "ImageExportForm"), parameters(parameters)
{

}

#define GT_CLASS_NAME "GTUtilsDialog::ImageExportFormFiller"
#define GT_METHOD_NAME "run"

void ImageExportFormFiller::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QCheckBox* export_msa_simple_overview = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "export_msa_simple_overview", dialog));
    GT_CHECK(export_msa_simple_overview, "export_msa_simple_overview is NULL");
    GTCheckBox::setChecked(os, export_msa_simple_overview, parameters.simpleOverviewChecked);

    QCheckBox* export_msa_graph_overview = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "export_msa_graph_overview", dialog));
    GT_CHECK(export_msa_graph_overview, "export_msa_graph_overview is NULL");
    GTCheckBox::setChecked(os, export_msa_graph_overview, parameters.graphOverviewChecked);

    QLineEdit* fileNameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "fileNameEdit", dialog));
    GT_CHECK(fileNameEdit, "fileNameEdit is NULL");
    GTLineEdit::setText(os, fileNameEdit, parameters.fileName);

    QComboBox* formatsBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "formatsBox", dialog));
    GT_CHECK(formatsBox, "formatsBox is NULL");
    GTComboBox::setIndexWithText(os, formatsBox, parameters.format);

    QDialogButtonBox* box = dialog->findChild<QDialogButtonBox*>("buttonBox");
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* ok = box->button(QDialogButtonBox::Ok);
    GT_CHECK(ok !=NULL, "ok button is NULL");
    GTWidget::click(os, ok);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
