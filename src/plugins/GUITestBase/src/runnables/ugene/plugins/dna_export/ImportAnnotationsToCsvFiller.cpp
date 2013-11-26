/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "ImportAnnotationsToCsvFiller.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"
#include "api/GTCheckBox.h"
#include "api/GTComboBox.h"
#include "api/GTSpinBox.h"
#include "api/GTLineEdit.h"
#include "api/GTPlainTextEdit.h"
#include "api/GTRadioButton.h"

#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QTableWidget>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ImportAnnotationsToCsvFiller"

ImportAnnotationsToCsvFiller::ImportAnnotationsToCsvFiller(U2OpStatus &_os, const QString &_fileToRead, const QString &_resultFile,
        ImportAnnotationsToCsvFiller::fileFormat _format, bool _addResultFileToProject, 
        bool _columnSeparator, const QString &_separator, bool _script, int _numberOfLines,
        const QString &_skipAllLinesStartsWith, bool _interpretMultipleAsSingle,
        bool _removeQuotesButton, const QString &_defaultAnnotationName, const RoleParameters& _roleParameters, GTGlobals::UseMethod method):
Filler(_os, "ImportAnnotationsFromCSVDialog"), fileToRead(_fileToRead), resultFile(_resultFile), format(_format), 
       addResultFileToProject(_addResultFileToProject), columnSeparator(_columnSeparator), script(_script),
       numberOfLines(_numberOfLines), separator(_separator),
       skipAllLinesStartsWith(_skipAllLinesStartsWith), interpretMultipleAsSingle(_interpretMultipleAsSingle),
       removeQuotesButton(_removeQuotesButton), defaultAnnotationName(_defaultAnnotationName), roleParameters(_roleParameters), useMethod(method) {

    comboBoxItems[BED] = "BED";
    comboBoxItems[EMBL] = "EMBL";
    comboBoxItems[FPKM] = "FPKM";
    comboBoxItems[GFF] = "GFF";
    comboBoxItems[GTF] = "GTF";
    comboBoxItems[Genbank] = "Genbank";
    comboBoxItems[Swiss_Prot] = "Swiss_Prot";
}

class RoleFiller : public Filler {
public:
    RoleFiller(U2OpStatus &os, ImportAnnotationsToCsvFiller::RoleParameter* _parameter)
        : Filler(os, "CSVColumnConfigurationDialog"), parameter(_parameter) {}

    virtual void run() {
        QWidget *dialog = QApplication::activeModalWidget();
        QCheckBox *addOffsetCheckBox = dialog->findChild<QCheckBox*>("startOffsetCheck");
        QSpinBox *addOffsetSpinBox = dialog->findChild<QSpinBox*>("startOffsetValue");
        QCheckBox *endPosCheckBox = dialog->findChild<QCheckBox*>("endInclusiveCheck");
        QCheckBox *strandMarkCheckBox = dialog->findChild<QCheckBox*>("complValueCheck");
        QLineEdit *markValueLineEdit = dialog->findChild<QLineEdit*>("complValueEdit");
        QLineEdit *qualifierLineEdit = dialog->findChild<QLineEdit*>("qualifierNameEdit");

        GTGlobals::sleep(300);
        ImportAnnotationsToCsvFiller::StartParameter* startP = dynamic_cast<ImportAnnotationsToCsvFiller::StartParameter*>(parameter);
        if (startP) {
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("startRB"));
            GTCheckBox::setChecked(os, addOffsetCheckBox, startP->addOffset);
            if (startP->addOffset) {
                GTSpinBox::setValue(os, addOffsetSpinBox, startP->numberOfBp);
            }
        }

        ImportAnnotationsToCsvFiller::EndParameter* endP = dynamic_cast<ImportAnnotationsToCsvFiller::EndParameter*>(parameter);
        if (endP) {
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("endRB"));
            GTCheckBox::setChecked(os, endPosCheckBox, endP->endPos);
        }
       
        ImportAnnotationsToCsvFiller::LengthParameter* lenghtP = dynamic_cast<ImportAnnotationsToCsvFiller::LengthParameter*>(parameter);
        if (lenghtP) {
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("lengthRB"));
        }
        
        ImportAnnotationsToCsvFiller::StrandMarkParameter* strandMarkP = dynamic_cast<ImportAnnotationsToCsvFiller::StrandMarkParameter*>(parameter);
        if (strandMarkP) {
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("complMarkRB"));
            GTCheckBox::setChecked(os, strandMarkCheckBox, strandMarkP->markValue);
            GTLineEdit::setText(os, markValueLineEdit, strandMarkP->markValueName);
        }

        ImportAnnotationsToCsvFiller::NameParameter* nameP = dynamic_cast<ImportAnnotationsToCsvFiller::NameParameter*>(parameter);
        if (nameP) {
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("nameRB"));
        }

        ImportAnnotationsToCsvFiller::QualifierParameter* qualP = dynamic_cast<ImportAnnotationsToCsvFiller::QualifierParameter*>(parameter);
        if (qualP) {
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("qualifierRB"));
            GTLineEdit::setText(os, qualifierLineEdit, qualP->name);
        }
        
        ImportAnnotationsToCsvFiller::IgnoreParameter* ignoreP = dynamic_cast<ImportAnnotationsToCsvFiller::IgnoreParameter*>(parameter);
        if (ignoreP) {
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("ignoreRB"));
        }

        delete parameter; 
        parameter = NULL;
        
        QPushButton *okButton = dialog->findChild<QPushButton*>("okButton");
        GTWidget::click(os, okButton);
    }
private:
    ImportAnnotationsToCsvFiller::RoleParameter *parameter;
};

class GTTableWidget {
public:
    static QPoint headerItemCenter(U2OpStatus&os, QTableWidget* w, int pos) {
        CHECK_SET_ERR_RESULT(w != NULL, "QTableWidget is NULL", QPoint());

        QTableWidgetItem *widgetItem =  w->item(0, pos);
        CHECK_SET_ERR_RESULT(widgetItem != NULL, "QTableWidgetItem is NULL", QPoint());

        QPoint itemPoint = w->visualItemRect(widgetItem).center();
        return w->mapToGlobal(itemPoint);
    }
};

#define GT_METHOD_NAME "run"
void ImportAnnotationsToCsvFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *readFileLineEdit = dialog->findChild<QLineEdit*>(QString::fromUtf8("readFileName"));
    GT_CHECK(readFileLineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, readFileLineEdit, fileToRead);

    QLineEdit *resultFileLineEdit = dialog->findChild<QLineEdit*>(QString::fromUtf8("saveFileName"));
    GT_CHECK(resultFileLineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, dialog->findChild<QLineEdit*>(QString::fromUtf8("saveFileName")), resultFile);

    QComboBox *comboBox = dialog->findChild<QComboBox*>();
    GT_CHECK(comboBox != NULL, "ComboBox not found");
    int index = comboBox->findText(comboBoxItems[format]);
    GT_CHECK(index != -1, QString("item \"%1\" in combobox not found").arg(comboBoxItems[format]));
    GTComboBox::setCurrentIndex(os, comboBox, index);

    QSpinBox *spinBox = dialog->findChild<QSpinBox *>();
    GT_CHECK(spinBox != NULL, "SpinBox not found");
    GTSpinBox::setValue(os, spinBox, numberOfLines);

    QCheckBox *checkBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("addToProjectCheck"));
    GTCheckBox::setChecked(os, checkBox, addResultFileToProject);

    if (columnSeparator) {
        QRadioButton* columnSeparator = qobject_cast<QRadioButton*>(GTWidget::findWidget(os, "columnSeparatorRadioButton", dialog));
        columnSeparator->setChecked(true);

        QLineEdit *separatorLineEdit = dialog->findChild<QLineEdit*>("separatorEdit");
        GT_CHECK(separatorLineEdit != NULL, "line edit not found");
        GTLineEdit::setText(os, separatorLineEdit, separator);
        }

    else {
        QRadioButton* scriptRadioButton = qobject_cast<QRadioButton*>(GTWidget::findWidget(os, "scriptRadioButton", dialog));
        scriptRadioButton->setChecked(true);
         }

    QLineEdit *firstLinesLineEdit = dialog->findChild<QLineEdit*>(QString::fromUtf8("prefixToSkipEdit"));
    GT_CHECK(firstLinesLineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, firstLinesLineEdit, skipAllLinesStartsWith);

    QCheckBox *separatorsModeCheckBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("separatorsModeCheck"));
    GTCheckBox::setChecked(os, separatorsModeCheckBox, interpretMultipleAsSingle);
    
    QCheckBox *removeQuotesCheckBox = dialog->findChild<QCheckBox*>(QString::fromUtf8("removeQuotesCheck"));
    GTCheckBox::setChecked(os, removeQuotesCheckBox, removeQuotesButton);
    
    QLineEdit *defaultAnnotationNameLineEdit = dialog->findChild<QLineEdit*>(QString::fromUtf8("defaultNameEdit"));
    GT_CHECK(defaultAnnotationNameLineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, defaultAnnotationNameLineEdit, defaultAnnotationName);

    QPushButton *previewButton = dialog->findChild<QPushButton*>("previewButton");
    GT_CHECK(previewButton != NULL, "DA button not found");
    GTWidget::click(os, previewButton);
    GTGlobals::sleep();

    QTableWidget *previewTable = dialog->findChild<QTableWidget*>("previewTable");
    foreach (RoleColumnParameter r, roleParameters) {
        GTUtilsDialog::waitForDialog(os, new RoleFiller(os, r.parameter));
        GTMouseDriver::moveTo(os, GTTableWidget::headerItemCenter(os, previewTable, r.column));
        GTMouseDriver::click(os);
    }
    QPushButton *runButton = dialog->findChild<QPushButton*>(QString::fromUtf8("runButton")); // KILL
    GT_CHECK(runButton != NULL, "Export button not found");
    GTWidget::click(os, runButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
