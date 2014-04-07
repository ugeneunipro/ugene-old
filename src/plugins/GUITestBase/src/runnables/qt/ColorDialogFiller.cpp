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

#include <api/GTKeyboardDriver.h>
#include <api/GTSpinBox.h>
#include "api/GTWidget.h"
#include "ColorDialogFiller.h"
#include <QColor>
#include <QColorDialog>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#else
#include <QtWidgets/QApplication>
#endif
#include <QWidget>
#include <QSpinBox>
namespace U2{

#define GT_CLASS_NAME "GTUtilsDialog::ColorDialog filler"
#define GT_METHOD_NAME "run"
void ColorDialogFiller::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog!=NULL, "dialog is NULL");

    QList<QSpinBox*> spinList = dialog->findChildren<QSpinBox*>();
    GTSpinBox::setValue(os,spinList.at(3),r,GTGlobals::UseKeyBoard);
    GTSpinBox::setValue(os,spinList.at(4),g,GTGlobals::UseKeyBoard);
    GTSpinBox::setValue(os,spinList.at(5),b,GTGlobals::UseKeyBoard);

    GTGlobals::sleep();
#ifdef Q_OS_MAC
    GTWidget::click(os, GTWidget::findButtonByText(os, "OK",dialog));
#else
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
#endif
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
