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

#include "AliasesDialogFiller.h"
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidget>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#endif

#include <primitives/GTWidget.h>
#include <primitives/GTTableView.h>
#include <drivers/GTMouseDriver.h>
#include <drivers/GTKeyboardDriver.h>

namespace U2{
using namespace HI;
#define GT_CLASS_NAME "GTUtilsDialog::StartupDialogFiller"
#define GT_METHOD_NAME "commonScenario"

void AliasesDialogFiller::commonScenario(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");
    GTGlobals::sleep(500);

    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"paramAliasesTableWidget",dialog));
    QMap<QPoint*, QString>::iterator i;
    for (i = map.begin(); i != map.end(); ++i){
        GTMouseDriver::moveTo(os,GTTableView::getCellPosition(os,table,i.key()->x(),i.key()->y()));
        GTMouseDriver::doubleClick(os);
        GTKeyboardDriver::keySequence(os, i.value());
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}

