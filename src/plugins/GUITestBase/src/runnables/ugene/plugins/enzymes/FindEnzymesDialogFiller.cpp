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

#include <QtGui/QApplication>
#include <QtGui/QTreeWidget>

#include "FindEnzymesDialogFiller.h"
#include "api/GTTreeWidget.h"
#include "api/GTWidget.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::DotPlotFiller"

FindEnzymesDialogFiller::FindEnzymesDialogFiller(U2OpStatus &os, const QStringList &enzymesToFind) :
    Filler(os, "FindEnzymesDialog"),
    enzymesToFind(enzymesToFind)
{
}

#define GT_METHOD_NAME "run"
void FindEnzymesDialogFiller::run() {
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(NULL != dialog, "activeModalWidget is NULL");

    QWidget *enzymesSelectorWidget = GTWidget::findWidget(os, "enzymesSelectorWidget");
    GT_CHECK(NULL != enzymesSelectorWidget, "enzymesSelectorWidget is NULL");

    GTWidget::click(os, GTWidget::findWidget(os, "selectNoneButton", enzymesSelectorWidget));

    QTreeWidget *enzymesTree = qobject_cast<QTreeWidget *>(GTWidget::findWidget(os, "tree", enzymesSelectorWidget));
    foreach (const QString& enzyme, enzymesToFind) {
        QTreeWidgetItem *item = GTTreeWidget::findItem(os, enzymesTree, enzyme);
        GTTreeWidget::checkItem(os, item);
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2