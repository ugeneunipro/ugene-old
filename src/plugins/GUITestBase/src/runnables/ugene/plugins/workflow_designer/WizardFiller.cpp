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

#include "WizardFiller.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "drivers/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "drivers/GTMouseDriver.h"
#include "api/GTFileDialog.h"
#include "api/GTRadioButton.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "../../src/corelibs/U2Designer/src/wizard/WDWizardPage.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QToolButton>
#include <QtGui/QWizard>
#include <QtGui/QScrollArea>
#include <QtGui/QComboBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWizard>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QComboBox>
#endif
#include <QDir>

#include "GTUtilsWizard.h"

namespace U2 {
using namespace HI;

#define GET_ACTIVE_DIALOG QWidget* dialog = QApplication::activeModalWidget();\
                          GT_CHECK(dialog, "activeModalWidget is NULL");\
                          QWizard* wizard = qobject_cast<QWizard*>(dialog);\
                          GT_CHECK(wizard, "activeModalWidget is not wizard");

#define GT_CLASS_NAME "GTUtilsDialog::WizardFiller"

#define GT_METHOD_NAME "commonScenario"

void WizardFiller::commonScenario(){
    GTGlobals::sleep();
    if(inputFiles.count() != 0 && !inputFiles.first().isEmpty()){
        GTUtilsWizard::setInputFiles(os, inputFiles);
    }
    GTUtilsWizard::setAllParameters(os, map);

    GTUtilsWizard::clickButton(os, GTUtilsWizard::Apply);
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

#undef GT_CLASS_NAME
}
