/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
        bool _removeQuotesButton, const QString &_defaultAnnotationName, GTGlobals::UseMethod method):
Filler(_os, "ImportAnnotationsFromCSVDialog"), fileToRead(_fileToRead), resultFile(_resultFile), format(_format), 
       addResultFileToProject(_addResultFileToProject), columnSeparator(_columnSeparator), separator(_separator), 
       script(_script), numberOfLines(_numberOfLines), 
       skipAllLinesStartsWith(_skipAllLinesStartsWith), interpretMultipleAsSingle(_interpretMultipleAsSingle),
       removeQuotesButton(_removeQuotesButton), defaultAnnotationName(_defaultAnnotationName), useMethod(method) {

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
    enum Role {Start, End, Length, strandMark, Name, Qualifier, Ignore};
    RoleFiller(U2OpStatus &os, Role _role, QString _q = "") : Filler(os, "CSVColumnConfigurationDialog"), role(_role), q(_q) {}

    virtual void run() {
        QWidget *dialog = QApplication::activeModalWidget();
        //GT_CHECK(dialog != NULL, "dialog not found");
        QLineEdit *qualifierLineEdit = dialog->findChild<QLineEdit*>("qualifierNameEdit");
        switch (role) {
        case Start:
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("startRB"));
            break;
        case End:
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("endRB"));
            break;
        case Length:
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("lengthRB"));
            break;
        case strandMark:
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("complMarkRB"));
            break;
        case Name:
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("nameRB"));
            break;
        case Qualifier:
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("qualifierRB"));
            GTLineEdit::setText(os, qualifierLineEdit, q);
            break;
        case Ignore:
            GTRadioButton::click(os, dialog->findChild<QRadioButton*>("ignoreRB"));
            break;
            }
        QPushButton *okButton = dialog->findChild<QPushButton*>("okButton"); // KILL
//        GT_CHECK(okButton != NULL, "Export button not found");
        GTWidget::click(os, okButton);
    }
private:
    Role role;
    QString q;
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
    GTLineEdit::setText(os, resultFileLineEdit, resultFile);

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
    GT_CHECK(readFileLineEdit != NULL, "line edit not found");
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

    GTUtilsDialog::waitForDialog(os, new RoleFiller(os, RoleFiller::Name));
    QTableWidgetItem *ke =  previewTable->item(0, 0);
    QPoint p = previewTable->visualItemRect(ke).center();
    QPoint globalP = previewTable->mapToGlobal(p);
    GTMouseDriver::moveTo(os, globalP);
    GTMouseDriver::click(os);

    GTUtilsDialog::waitForDialog(os, new RoleFiller(os, RoleFiller::Start));
    QTableWidgetItem *ke1 =  previewTable->item(0, 1);
    QPoint p1 = previewTable->visualItemRect(ke1).center();
    QPoint globalP1 = previewTable->mapToGlobal(p1);
    GTMouseDriver::moveTo(os, globalP1);
    GTMouseDriver::click(os);

    GTUtilsDialog::waitForDialog(os, new RoleFiller(os, RoleFiller::End));
    QTableWidgetItem *ke2 =  previewTable->item(0, 2);
    QPoint p2 = previewTable->visualItemRect(ke2).center();
    QPoint globalP2 = previewTable->mapToGlobal(p2);
    GTMouseDriver::moveTo(os, globalP2);
    GTMouseDriver::click(os);

    GTUtilsDialog::waitForDialog(os, new RoleFiller(os, RoleFiller::Qualifier, "Qual"));
    QTableWidgetItem *ke3 =  previewTable->item(0, 3);
    QPoint p3 = previewTable->visualItemRect(ke3).center();
    QPoint globalP3 = previewTable->mapToGlobal(p3);
    GTMouseDriver::moveTo(os, globalP3);
    GTMouseDriver::click(os);
    
    QPushButton *runButton = dialog->findChild<QPushButton*>(QString::fromUtf8("runButton")); // KILL
    GT_CHECK(runButton != NULL, "Export button not found");
    GTWidget::click(os, runButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
