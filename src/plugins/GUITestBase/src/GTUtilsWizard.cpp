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


#include <QApplication>
#include <QLabel>
#include <QScrollArea>
#include <QToolButton>
#include <QWizard>

#include "api/GTComboBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTFileDialog.h"
#include "api/GTLineEdit.h"
#include "api/GTSpinBox.h"
#include "api/GTTabWidget.h"
#include "api/GTWidget.h"

#include "GTUtilsWizard.h"

namespace U2 {
QMap<QString, GTUtilsWizard::WizardButton> GTUtilsWizard::initButtonMap(){
   QMap<QString, WizardButton> result;
   result.insert("&Next >", Next);
   result.insert("< &Back", Back);
   result.insert("Apply", Apply);
   result.insert("Run", Run);
   result.insert("Cancel", Cancel);
   result.insert("Defaults", Defaults);
   result.insert("Setup", Setup);
   return result;
}
const QMap<QString, GTUtilsWizard::WizardButton> GTUtilsWizard::buttonMap = GTUtilsWizard::initButtonMap();

#define GT_CLASS_NAME "GTUtilsWizard"

#define GT_METHOD_NAME "setInputFiles"
void GTUtilsWizard::setInputFiles(U2OpStatus &os, QList<QStringList> inputFiles){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "wizard not found");
    int i = 0;
    foreach (QStringList datasetFiles, inputFiles) {
        QTabWidget* tabWidget = dialog->findChild<QTabWidget*>();
        GT_CHECK(tabWidget != NULL, "tabWidget not found");
        GTTabWidget::setCurrentIndex(os, tabWidget, i);
        foreach (QString s, datasetFiles) {
            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, s));
            QList<QWidget*> adds = dialog->findChildren<QWidget*>("addFileButton");
            foreach (QWidget* add, adds) {
                if(add->isVisible()){
                    GTWidget::click(os, add);
                    break;
                }
            }
            //GTWidget::click(os, GTWidget::findWidget(os, "addFileButton", dialog));
        }
        i++;
        //QTabWidget* tabWidget = GTWidget::
        //GTWidget::getAllWidgetsInfo(os, dialog);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setAllParameters"
void GTUtilsWizard::setAllParameters(U2OpStatus &os, QMap<QString, QVariant> map){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");
    QWizard* wizard = qobject_cast<QWizard*>(dialog);
    GT_CHECK(wizard, "activeModalWidget is not wizard");

    QWidget* nextButton = GTWidget::findButtonByText(os, "&Next >", wizard);

    while(nextButton != NULL && nextButton->isVisible()){

        QMap<QString, QVariant>::iterator iter = map.begin();
        while(iter != map.end()){
            QString k = iter.key();
            QWidget* w = GTWidget::findWidget(os, iter.key() + " widget", wizard->currentPage(), GTGlobals::FindOptions(false));
            if(w != NULL){
                QToolButton* showHide = GTWidget::findExactWidget<QToolButton*>(os, "showHideButton", dialog, GTGlobals::FindOptions(false));
                if(showHide != NULL && showHide->text() == "+"){
                    GTWidget::click(os, showHide);
                }
                QScrollArea* area = wizard->currentPage()->findChild<QScrollArea*>();
                if(area != NULL){
                    area->ensureWidgetVisible(w);
                }
                setValue(os, w, iter.value());
                iter = map.erase(iter);

            }else{
                ++iter;
            }
        }
        nextButton = GTWidget::findButtonByText(os, "&Next >", wizard, GTGlobals::FindOptions(false));
        if(nextButton != NULL && nextButton->isVisible()){
            GTWidget::click(os, nextButton);
        }
    }

}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setParameter"
void GTUtilsWizard::setParameter(U2OpStatus &os, QString parName, QVariant parValue){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");
    QWizard* wizard = qobject_cast<QWizard*>(dialog);
    GT_CHECK(wizard, "activeModalWidget is not wizard");

    QToolButton* showHide = GTWidget::findExactWidget<QToolButton*>(os, "showHideButton", dialog, GTGlobals::FindOptions(false));
    if(showHide != NULL && showHide->text() == "+"){
        GTWidget::click(os, showHide);
    }

    QWidget* w = GTWidget::findWidget(os, parName + " widget", dialog);

    QScrollArea* area = wizard->currentPage()->findChild<QScrollArea*>();
    if(area != NULL){
        area->ensureWidgetVisible(w);
    }

    setValue(os, w, parValue);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setParameter"
QVariant GTUtilsWizard::getParameter(U2OpStatus &os, QString parName){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK_RESULT(dialog, "activeModalWidget is NULL", QVariant());
    QWizard* wizard = qobject_cast<QWizard*>(dialog);
    GT_CHECK_RESULT(wizard, "activeModalWidget is not wizard", QVariant());

    QToolButton* showHide = GTWidget::findExactWidget<QToolButton*>(os, "showHideButton", dialog, GTGlobals::FindOptions(false));
    if(showHide != NULL && showHide->text() == "+"){
        GTWidget::click(os, showHide);
    }

    QWidget* w = GTWidget::findWidget(os, parName + " widget", dialog);

    QComboBox* combo = qobject_cast<QComboBox*>(w);
    if(combo != NULL){
        return QVariant(combo->currentText());
    }
    QSpinBox* spin = qobject_cast<QSpinBox*>(w);
    if(spin != NULL){
        return QVariant(spin->value());
    }
    QDoubleSpinBox* doubleSpin = qobject_cast<QDoubleSpinBox*>(w);
    if(doubleSpin != NULL){
        return QVariant(doubleSpin->value());
    }
    QLineEdit* line = qobject_cast<QLineEdit*>(w);
    if(line != NULL){
        return QVariant(line->text());
    }
    GT_CHECK_RESULT(false, QString("unsupported widget class: %1").arg(w->metaObject()->className()), QVariant());
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "setValue"
void GTUtilsWizard::setValue(U2OpStatus &os, QWidget *w, QVariant value){
    QComboBox* combo = qobject_cast<QComboBox*>(w);
    if(combo != NULL){
        GTComboBox::setIndexWithText(os, combo, value.toString());
        return;
    }
    QSpinBox* spin = qobject_cast<QSpinBox*>(w);
    if(spin != NULL){
        bool ok;
        int val = value.toInt(&ok);
        GT_CHECK(ok, "spin box needs int value");
        GTSpinBox::setValue(os, spin, val, GTGlobals::UseKeyBoard);
        return;
    }
    QDoubleSpinBox* doubleSpin = qobject_cast<QDoubleSpinBox*>(w);
    if(doubleSpin != NULL){
        bool ok;
        int val = value.toDouble(&ok);
        GT_CHECK(ok, "double spin box needs double value");
        GTDoubleSpinbox::setValue(os, doubleSpin, val, GTGlobals::UseKeyBoard);
        return;
    }
    QLineEdit* line = qobject_cast<QLineEdit*>(w);
    if(line != NULL){
        GTLineEdit::setText(os, line, value.toString());
        return;
    }
    GT_CHECK(false, QString("unsupported widget class: %1").arg(w->metaObject()->className()));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickButton"
void GTUtilsWizard::clickButton(U2OpStatus &os, WizardButton button){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog!=NULL, "activeModalWidget is NULL");

    QWidget* w = GTWidget::findButtonByText(os, buttonMap.key(button), dialog);
    GTGlobals::sleep(500);
    GTWidget::click(os, w);

}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPageTitle"
QString GTUtilsWizard::getPageTitle(U2OpStatus &os){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK_RESULT(dialog!=NULL, "activeModalWidget is NULL", QString());
    QWizard* wizard = qobject_cast<QWizard*>(dialog);
    GT_CHECK_RESULT(wizard, "activeModalWidget is not wizard", QString());

    QLabel* pageTitle = GTWidget::findExactWidget<QLabel*>(os, "pageTitle", wizard->currentPage());
    return pageTitle->text();
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
