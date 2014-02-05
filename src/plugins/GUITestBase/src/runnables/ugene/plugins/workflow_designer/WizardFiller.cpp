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

#include "WizardFiller.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTMouseDriver.h"
#include "api/GTFileDialog.h"
#include "api/GTRadioButton.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "../../src/corelibs/U2Designer/src/wizard/WDWizardPage.h"

#include <QtGui/QApplication>
#include <QtGui/QRadioButton>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QWizard>
#include <QtGui/QScrollArea>
#include <QtGui/QComboBox>
#include <QDir>

namespace U2 {
#define GET_ACTIVE_DIALOG QWidget* dialog = QApplication::activeModalWidget();\
                          GT_CHECK(dialog, "activeModalWidget is NULL");\
                          QWizard* wizard = qobject_cast<QWizard*>(dialog);\
                          GT_CHECK(wizard, "activeModalWidget is not wizard");

#define GT_CLASS_NAME "GTUtilsDialog::WizardFiller"
#define GT_METHOD_NAME "setParameters"
void WizardFiller::setParameters(U2OpStatus &os, pairValList list,int i){
    GET_ACTIVE_DIALOG
    QScrollArea* scroll = wizard->currentPage()->findChild<QScrollArea*>();
    QList<QLabel*> labelList = wizard->currentPage()->findChildren<QLabel*>();
    QList<QWidget*> widgetList = wizard->currentPage()->findChildren<QWidget*>();

    QToolButton* expand = getExpandButton(os);
    if (expand)
        GTWidget::click(os,expand);
    foreach (pairValLabel pair, list){
    if (!(pair.val->page==-1) && !(pair.val->page==i))
        continue;
    foreach(QLabel* label, labelList){
        if(label->text().toLower()==pair.label.toLower()){
            if(scroll)
                scroll->ensureWidgetVisible(label);

            QList<QWidget*> yList;
            foreach (QWidget* w, widgetList){
                if(abs(w->mapTo(wizard,w->rect().center()).y() - label->mapTo(wizard,label->rect().center()).y())==0){
                    yList.append(w);
                }
            }

            lineEditValue* strVal = dynamic_cast<lineEditValue*>(pair.val);
            if(strVal){
                QLineEdit* lineEd = NULL;
                foreach(QWidget* w, yList){
                    QLineEdit* line = dynamic_cast<QLineEdit*>(w);
                    if (line)
                        lineEd = line;
                }

                GTLineEdit::setText(os,lineEd, actualizePath(strVal));
                //lineEd->setText(strVal->lineValue);
                pair.val->isSet = true;
            }
            spinBoxValue* spinVal = dynamic_cast<spinBoxValue*>(pair.val);
            if(spinVal){
                QSpinBox* spinBox = NULL;
                foreach(QWidget* w, yList){
                    QSpinBox* spin = dynamic_cast<QSpinBox*>(w);
                    if (spin)
                        spinBox = spin;
                }
                GTSpinBox::setValue(os,spinBox,spinVal->spinValue);
                pair.val->isSet = true;
            }
            doubleSpinBoxValue* doubleSpinVal = dynamic_cast<doubleSpinBoxValue*>(pair.val);
            if(doubleSpinVal){
                QDoubleSpinBox* doubleSpinBox = NULL;
                foreach(QWidget* w, yList){
                    QDoubleSpinBox* spin = dynamic_cast<QDoubleSpinBox*>(w);
                    if (spin)
                        doubleSpinBox = spin;
                }
                GTDoubleSpinbox::setValue(os, doubleSpinBox, doubleSpinVal->spinValue);
                pair.val->isSet = true;
            }
            ComboBoxValue* comboVal = dynamic_cast<ComboBoxValue*>(pair.val);
            if(comboVal){
                QComboBox* comboBox = NULL;
                foreach(QWidget* w, yList){
                    QComboBox* combo = dynamic_cast<QComboBox*>(w);
                    if (combo)
                        comboBox = combo;
                }
                GTComboBox::setCurrentIndex(os, comboBox,comboVal->comboValue);
                pair.val->isSet = true;
            }
        }
    }
    bool isSet = pair.val->page==-1 || (pair.val->page==i && pair.val->isSet);
    GT_CHECK(isSet, QString("label %1 not found at page %2").arg(pair.label).arg(i));
    }

}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "WizardFiller::fill"
void WizardFiller::fill(U2OpStatus &os, pairValList list){
    GET_ACTIVE_DIALOG
    QPushButton* next = getNextButton(os);
    QPushButton* finish = getFinishButton(os);

    for(int i=0;i<20;i++){
       if(getExpandButton(os))
           GTWidget::click(os,getExpandButton(os));

       setParameters(os,list,i);
       GTGlobals::sleep(500);
       if (finish->isEnabled())
           break;
       else
           GTWidget::click(os, next);
    }

    foreach(pairValLabel pair, list){
        GT_CHECK(pair.val->isSet, QString("Value \"%1\" was not set. Probably, %1 label was not found").arg(pair.label));
    }
    GTWidget::click(os, getFinishButton(os));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "WizardFiller::actualizePath"
QString WizardFiller::actualizePath(lineEditValue* val){
    if(!val->isPath)
        return val->lineValue;
    QString path;
    path = QDir::cleanPath(QDir::currentPath() + "/" + val->lineValue);
    if (path.at(path.count() - 1) != '/') {
        path += '/';
    }
    return path;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "WizardFiller::getButtonByText"
QPushButton* WizardFiller::getButtonByText(U2OpStatus &os, QString text){
    QPushButton* nextButton=NULL;

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK_RESULT(dialog, "activeModalWidget is NULL", NULL);
    GTGlobals::sleep(500);

    QList<QPushButton*> ButtonList=dialog->findChildren<QPushButton*>();
    foreach (QPushButton* but, ButtonList){
        if(but->text() == text)
            nextButton = but;
    }
    return nextButton;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "WizardFiller::getExpandButton"
QToolButton* WizardFiller::getExpandButton(U2OpStatus &os){
    QToolButton* expandButton = NULL;
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK_RESULT(dialog, "activeModalWidget is NULL",NULL);
    QWizard* wizard = qobject_cast<QWizard*>(dialog);
    GT_CHECK_RESULT(wizard, "activeModalWidget is not of wizard type",NULL);

    QList<QWidget*> widList = wizard->currentPage()->findChildren<QWidget*>();
    QList<QToolButton*> plusList;
    foreach(QWidget* w, widList){
        QToolButton* but = qobject_cast<QToolButton*>(w);
        if (but && but->text()=="+" && abs(but->rect().width()-19)<2)
            plusList.append(but);
    }
    // there can be one or more '+' buttons at wizard page which are invisiable. TODO:detect them
    if (!plusList.isEmpty())
        expandButton = plusList.takeLast();
    return expandButton;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "WizardFiller::getNextButton"
QPushButton* WizardFiller::getNextButton(U2OpStatus &os){
    QPushButton* next = getButtonByText(os, "&Next >");
    GT_CHECK_RESULT(next, "Next button not found",NULL);
    return next;
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "WizardFiller::getFinishButton"
QPushButton* WizardFiller::getFinishButton(U2OpStatus &os){
    QPushButton* finish = getButtonByText(os, "Apply");
    GT_CHECK_RESULT(finish, "finish button not found",NULL);
    return finish;
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "WizardFiller::getCancelButton"
QPushButton* WizardFiller::getCancelButton(U2OpStatus &os){
    QPushButton* Cancel = getButtonByText(os, "Cancel");
    GT_CHECK_RESULT(Cancel, "Cancel button not found",NULL);
    return Cancel;
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "WizardFiller::getSetupButton"
QPushButton* WizardFiller::getSetupButton(U2OpStatus &os){
    QPushButton* Setup = getButtonByText(os, "Setup");
    GT_CHECK_RESULT(Setup, "Setup button not found",NULL);
    return Setup;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ConfigureTuxedoWizardFiller"
#define GT_METHOD_NAME "run"
void ConfigureTuxedoWizardFiller::run() {
    GET_ACTIVE_DIALOG
    GTGlobals::sleep(500);
    QMap<analysis_type,QString> anMap;
    QMap<reads_type,QString> rMap;

    anMap[full] = "Full Tuxedo";
    anMap[single_sample] = "single-sample tuxedo";
    anMap[no_new_transkripts] = "no-new-transcripts";

    rMap[singleReads] = "Single-end reads";
    rMap[pairedReads] = "Paired-end reads";

    GTRadioButton::click(os,GTRadioButton::getRadioButtonByText(os,anMap[analys],dialog));
    GTRadioButton::click(os,GTRadioButton::getRadioButtonByText(os,rMap[reads],dialog));

    GTWidget::click(os, getSetupButton(os));

}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::TuxedoWizardFiller"
#define GT_METHOD_NAME "run"
void TuxedoWizardFiller::run(){

    GET_ACTIVE_DIALOG
    GTGlobals::sleep(1000);

    QToolButton* addFile;
    QList<QToolButton*> toolList= wizard->currentPage()->findChildren<QToolButton*>();
    QList<QToolButton*> addButtonList;
    foreach(QToolButton* but, toolList){
        if(but->objectName()=="addFileButton" && but->isVisible())
            addButtonList.append(but);
    }
    addFile = addButtonList.takeLast();

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, path1, name1);
    GTUtilsDialog::waitForDialog(os, ob);
    if(addButtonList.count()==1){//for paired reads
        GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    }
    GTGlobals::sleep(500);

    GTWidget::click(os,addFile);

    ob = new GTFileDialogUtils(os, path2, name2);
    GTUtilsDialog::waitForDialog(os, ob);
    if(addButtonList.count()==1){//for paired reads
        GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    }

    QTabWidget* tabWidget = wizard->currentPage()->findChild<QTabWidget*>();
    QPoint p = tabWidget->mapToGlobal(tabWidget->rect().topLeft());
    p.setX(p.x()+150);
    p.setY(p.y()+20);
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::click(os);
    GTWidget::click(os,addFile);

    fill(os,list);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
