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

#include <QInputDialog>
#include <QMessageBox>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "DatasetsListWidget.h"

namespace U2 {

DatasetsListWidget::DatasetsListWidget(QWidget *parent)
: QWidget(parent)
{
    setupUi(this);

    QToolButton *newTabButton = new QToolButton(this);
    tabs->setCornerWidget(newTabButton, Qt::TopLeftCorner);
    newTabButton->setCursor(Qt::ArrowCursor);
    newTabButton->setAutoRaise(true);
    newTabButton->setText("+");
    newTabButton->setToolTip(tr("Add dataset"));
    QIcon addIcon = QIcon(QString(":U2Designer/images/add.png"));
    newTabButton->setIcon(addIcon);
    connect(newTabButton, SIGNAL(clicked()), SLOT(sl_newDataset()));
    connect(tabs, SIGNAL(tabCloseRequested(int)), SLOT(sl_deleteDataset(int)));
}

void DatasetsListWidget::appendDataset(const QString &name, DatasetWidget *page) {
    int lastPos = tabs->count();
    tabs->insertTab(lastPos, page, name);
    connect(page, SIGNAL(si_datasetRenamed(const QString &)), SLOT(sl_renameDataset(const QString &)));
}

void DatasetsListWidget::sl_deleteDataset(int idx) {
    DatasetWidget *page = dynamic_cast<DatasetWidget*>(tabs->widget(idx));
    SAFE_POINT(NULL != page, "NULL page wigdet", );

    tabs->removeTab(idx);
    page->deleteDataset();
}

void DatasetsListWidget::sl_newDataset() {
    QString error;
    do {
        bool ok = false;
        QString text = QInputDialog::getText(this,
            tr("Enter Dataset Name"),
            tr("New dataset name:"),
            QLineEdit::Normal,
            "", &ok);
        if (!ok) {
            return;
        }
        if (!text.isEmpty()) {
            U2OpStatusImpl os;
            emit si_addDataset(text, os);
            error = os.getError();
        }
        if (!error.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), error);
        }
    } while (!error.isEmpty());
}

void DatasetsListWidget::sl_renameDataset(const QString &newName) {
    tabs->setTabText(tabs->currentIndex(), newName);
}

} // U2
