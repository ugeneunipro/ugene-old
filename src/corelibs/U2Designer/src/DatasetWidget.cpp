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

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/LastUsedDirHelper.h>

#include "DatasetWidget.h"

namespace U2 {

DatasetWidget::DatasetWidget(const QString &name, QWidget *parent)
: QWidget(parent)
{
    setupUi(this);
    nameLabel->setText(name);

    addButton->hide();
    reset();
    QIcon fileIcon = QIcon(QString(":U2Designer/images/file.png"));
    QIcon dirIcon = QIcon(QString(":U2Designer/images/directory.png"));
    QIcon addIcon = QIcon(QString(":U2Designer/images/add.png"));
    QIcon deleteIcon = QIcon(QString(":U2Designer/images/exit.png"));
    QIcon upIcon = QIcon(QString(":U2Designer/images/up.png"));
    QIcon downIcon = QIcon(QString(":U2Designer/images/down.png"));

    addFileButton->setIcon(fileIcon);
    addDirButton->setIcon(dirIcon);
    addButton->setIcon(addIcon);
    deleteButton->setIcon(deleteIcon);
    upButton->setIcon(upIcon);
    downButton->setIcon(downIcon);

    connect(inputUrlEdit, SIGNAL(textChanged(const QString &)), SLOT(sl_textChanged(const QString &)));
    connect(addFileButton, SIGNAL(clicked()), SLOT(sl_addFileButton()));
    connect(addDirButton, SIGNAL(clicked()), SLOT(sl_addDirButton()));
    connect(addButton, SIGNAL(clicked()), SLOT(sl_addButton()));
    connect(inputUrlEdit, SIGNAL(returnPressed()), SLOT(sl_addButton()));
    connect(downButton, SIGNAL(clicked()), SLOT(sl_downButton()));
    connect(upButton, SIGNAL(clicked()), SLOT(sl_upButton()));
    connect(deleteButton, SIGNAL(clicked()), SLOT(sl_deleteButton()));
    connect(renameButton, SIGNAL(clicked()), SLOT(sl_renameButton()));

    connect(itemsArea, SIGNAL(itemSelectionChanged()), SLOT(sl_itemChecked()));
}

void DatasetWidget::addUrlItem(UrlItem *urlItem) {
    urlItem->setParent(itemsArea);
    itemsArea->addItem(urlItem);
}

void DatasetWidget::deleteDataset() {
    emit si_datasetDeleted();
}

void DatasetWidget::sl_addButton() {
    addUrl(inputUrlEdit->text());
}

void DatasetWidget::sl_addFileButton() {
    LastUsedDirHelper lod("dataset_file");
    QStringList files = QFileDialog::getOpenFileNames(NULL, tr("Select file"), lod.dir);
    foreach (const QString &file, files) {
        lod.url = file;
        addUrl(file);
    }
}

void DatasetWidget::sl_addDirButton() {
    LastUsedDirHelper lod("dataset_dir");
    QString dir = QFileDialog::getExistingDirectory(NULL, tr("Select a directory"), lod.dir);
    if (!dir.isEmpty()) {
        lod.dir = dir;
        addUrl(dir);
    }
}

void DatasetWidget::sl_textChanged(const QString &text) {
    if (text.isEmpty()) {
        addButton->hide();
        addFileButton->show();
        addDirButton->show();
    } else {
        addButton->show();
        addFileButton->hide();
        addDirButton->hide();
    }
}

void DatasetWidget::addUrl(const QString &url) {
    U2OpStatusImpl os;
    emit si_addUrl(url, os);
    if (os.hasError()) {
        QMessageBox::critical(this, tr("Error"), os.getError());
    } else {
        inputUrlEdit->setText("");
    }
}

void DatasetWidget::sl_itemChecked() {
    reset();
    if (itemsArea->selectedItems().size() > 0) {
        deleteButton->setEnabled(true);
        bool firstSelected = itemsArea->item(0)->isSelected();
        bool lastSelected = itemsArea->item(itemsArea->count() - 1)->isSelected();
        upButton->setEnabled(!firstSelected);
        downButton->setEnabled(!lastSelected);
    }
    if (1 == itemsArea->selectedItems().size()) {
        QListWidgetItem *item = itemsArea->selectedItems().first();
        UrlItem *urlItem = dynamic_cast<UrlItem*>(item);
        CHECK(NULL != urlItem, );
        QWidget *options = urlItem->getOptionsWidget();
        if (NULL != options) {
            showOptions(options);
        }
    }
}

void DatasetWidget::reset() {
    deleteButton->setEnabled(false);
    upButton->setEnabled(false);
    downButton->setEnabled(false);
    hideOptions();
}

void DatasetWidget::showOptions(QWidget *options) {
    configLayout->insertWidget(1, options);
    options->show();
}

void DatasetWidget::hideOptions() {
    if (3 == configLayout->count()) {
        QLayoutItem *item = configLayout->itemAt(1);
        configLayout->removeItem(item);
        item->widget()->hide();
        delete item;
    }
}

void DatasetWidget::sl_downButton() {
    CHECK(itemsArea->selectedItems().size() > 0, );

    for (int pos=itemsArea->count() - 2; pos >= 0; pos--) { // without last item
        if (itemsArea->item(pos)->isSelected()) {
            QListWidgetItem *item = itemsArea->takeItem(pos);
            itemsArea->insertItem(pos+1, item);
            item->setSelected(true);
            emit si_replaceUrl(dynamic_cast<UrlItem*>(item), pos+1);
        }
    }
}

void DatasetWidget::sl_upButton() {
    CHECK(itemsArea->selectedItems().size() > 0, );

    for (int pos=1; pos < itemsArea->count(); pos++) { // without first item
        if (itemsArea->item(pos)->isSelected()) {
            QListWidgetItem *item = itemsArea->takeItem(pos);
            itemsArea->insertItem(pos-1, item);
            item->setSelected(true);
            emit si_replaceUrl(dynamic_cast<UrlItem*>(item), pos-1);
        }
    }
}

void DatasetWidget::sl_deleteButton() {
    foreach (QListWidgetItem *item, itemsArea->selectedItems()) {
        dynamic_cast<UrlItem*>(item)->deleteItem();
        itemsArea->takeItem(itemsArea->row(item));
    }
}

void DatasetWidget::sl_renameButton() {
    bool error = false;
    do {
        bool ok = false;
        QString text = QInputDialog::getText(this,
            tr("Rename Dataset"),
            tr("New dataset name:"),
            QLineEdit::Normal,
            nameLabel->text(), &ok);
        if (!ok) {
            return;
        }
        U2OpStatusImpl os;
        emit si_renameDataset(text, os);

        if (os.hasError()) {
            QMessageBox::critical(this, tr("Error"), os.getError());
        } else {
            nameLabel->setText(text);
            emit si_datasetRenamed(text);
        }
        error = os.hasError();
    } while (error);
}

} // U2
