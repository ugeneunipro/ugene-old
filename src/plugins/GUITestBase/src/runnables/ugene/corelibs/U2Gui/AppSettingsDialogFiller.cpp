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
#include "AppSettingsDialogFiller.h"

#include "api/GTWidget.h"
#include "api/GTTreeWidget.h"
#include "api/GTMouseDriver.h"
#include "api/GTComboBox.h"

#include <QtGui/QApplication>
#include <QtGui/QTreeWidget>
#include <QtGui/QComboBox>

namespace U2{

#define GT_CLASS_NAME "GTUtilsDialog::AppSettingsDialogFiller"
#define GT_METHOD_NAME "run"
void AppSettingsDialogFiller::run(){

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QTreeWidget* tree = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"tree"));
    GT_CHECK(tree, "tree widger not found");

    QList<QTreeWidgetItem*> items = GTTreeWidget::getItems(tree->invisibleRootItem());
    foreach (QTreeWidgetItem* item,items){
        if(item->text(0)=="  Workflow Designer"){
            GTMouseDriver::moveTo(os,GTTreeWidget::getItemCenter(os,item));
            GTMouseDriver::click(os);
        }
    }
    QComboBox *styleCombo = qobject_cast<QComboBox*>(GTWidget::findWidget(os,"styleCombo",dialog));
    GTComboBox::setCurrentIndex(os,styleCombo,itemStyle);
    GTWidget::click(os,GTWidget::findWidget(os,"okButton"));
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
