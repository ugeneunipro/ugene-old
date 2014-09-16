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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QListWidget>
#else
#include <QtWidgets/QListWidget>
#endif

#include "api/GTMouseDriver.h"

#include "GTListWidget.h"

namespace U2{
#define GT_CLASS_NAME "GTListWidget"

#define GT_METHOD_NAME "click"
void GTListWidget::click(U2OpStatus &os, QListWidget *listWidget, const QString &text){
    QList<QListWidgetItem*> list = listWidget->findItems(text, Qt::MatchExactly);
    GT_CHECK(!list.isEmpty(), QString("item %1 not found").arg(text));

    QListWidgetItem* item = list.first();
    listWidget->scrollToItem(item);

    QRect r = listWidget->visualItemRect(item);
    QPoint global = listWidget->mapToGlobal(r.center());
    GTMouseDriver::moveTo(os, global);
    GTMouseDriver::click(os);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME
}
