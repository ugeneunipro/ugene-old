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
#include <QMenu>
#include <QMessageBox>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "DatasetsListWidget.h"

namespace U2 {

DatasetsListWidget::DatasetsListWidget(QWidget *parent)
: QWidget(parent)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    tabs = new DatasetsTabWidget(this);
    l->addWidget(tabs);

    QToolButton *newTabButton = new QToolButton(this);
    tabs->setCornerWidget(newTabButton, Qt::TopRightCorner);
    newTabButton->setCursor(Qt::ArrowCursor);
    newTabButton->setAutoRaise(true);
    newTabButton->setText("+");
    newTabButton->setToolTip(tr("Add dataset"));
    QIcon addIcon = QIcon(QString(":U2Designer/images/add.png"));
    newTabButton->setIcon(addIcon);
    connect(newTabButton, SIGNAL(clicked()), SLOT(sl_newDataset()));
    connect(tabs, SIGNAL(tabCloseRequested(int)), SLOT(sl_deleteDataset(int)));
    connect(tabs, SIGNAL(si_contextMenu(const QPoint &, int)), SLOT(sl_contextMenu(const QPoint &, int)));
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

QString DatasetsListWidget::getTip() const {
    QStringList names;
    for (int i=0; i<tabs->count(); i++) {
        names << tabs->tabText(i);
    }
    int idx = names.count();
    QString result;
    do {
        idx++;
        result = QString("Dataset %1").arg(idx);
    } while (names.contains(result));

    return result;
}

void DatasetsListWidget::sl_newDataset() {
    QString error;
    QString text = getTip();
    do {
        bool ok = false;
        text = QInputDialog::getText(this,
            tr("Enter Dataset Name"),
            tr("New dataset name:"),
            QLineEdit::Normal,
            text, &ok);
        if (!ok) {
            return;
        }
        U2OpStatusImpl os;
        emit si_addDataset(text, os);
        error = os.getError();
        if (!error.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), error);
        }
    } while (!error.isEmpty());
}

void DatasetsListWidget::sl_renameDataset(const QString &newName) {
    int idx = tabs->indexOf(dynamic_cast<DatasetWidget*>(sender()));
    CHECK(-1 != idx, );
    tabs->setTabText(idx, newName);
}

void DatasetsListWidget::sl_renameDataset() {
    QAction *a = dynamic_cast<QAction*>(sender());
    CHECK(NULL != a, );

    int idx = a->property("idx").toInt();
    DatasetWidget *page = dynamic_cast<DatasetWidget*>(tabs->widget(idx));
    CHECK(NULL != page, );

    page->renameDataset(tabs->tabText(idx));
}

void DatasetsListWidget::sl_contextMenu(const QPoint &p, int idx) {
    QMenu menu;
    QAction *renameAction = new QAction(tr("Rename dataset"), &menu);
    renameAction->setProperty("idx", idx);
    connect(renameAction, SIGNAL(triggered()), SLOT(sl_renameDataset()));
    menu.addAction(renameAction);
    menu.exec(p);
}

/************************************************************************/
/* DatasetsTabWidget */
/************************************************************************/
DatasetsTabWidget::DatasetsTabWidget(QWidget *parent)
: QTabWidget(parent)
{
    setUsesScrollButtons(true);
    setDocumentMode(true);
    setTabsClosable(true);
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_contextMenu(const QPoint &)));
}

void DatasetsTabWidget::sl_contextMenu(const QPoint &p) {
    int idx = tabBar()->tabAt(p);
    if (-1 != idx) {
        emit si_contextMenu(tabBar()->mapToGlobal(p), idx);
    }
}

} // U2
