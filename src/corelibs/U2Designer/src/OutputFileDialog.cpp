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

#include <U2Core/U2SafePoints.h>

#include <U2Gui/SuggestCompleter.h>

#include <U2Lang/RunFileSystem.h>

#include "OutputFileDialog.h"

namespace U2 {

static const QString OUT_DIR = QObject::tr("Output directory");

OutputFileDialog::OutputFileDialog(RunFileSystem *_rfs, bool _saveDir, CompletionFiller *filler, QWidget *parent)
: QDialog(parent), rfs(_rfs), saveDir(_saveDir), saveToFileSystem(false)
{
    setupUi(this);
    addDirButton->setIcon(QIcon(":U2Designer/images/add_directory.png"));
    absolutePathButton->setIcon(QIcon(":U2Designer/images/outside.png"));
    if (saveDir) {
        nameWidget->setVisible(false);
        setWindowTitle(tr("Save a directory"));
    } else {
        setWindowTitle(tr("Save a file"));
        if (NULL != filler) {
            new BaseCompleter(filler, nameEdit);
        }
    }
    updateFocus();

    model = new RFSTreeModel(rfs->getRoot(), saveDir, this);
    selectionModel = new QItemSelectionModel(model);
    treeView->setModel(model);
    treeView->setSelectionModel(selectionModel);
    treeView->expandAll();
    selectionModel->select(model->index(0, 0, QModelIndex()), QItemSelectionModel::Select);
    updateSaveButton();

    connect(nameEdit, SIGNAL(textEdited(const QString &)), SLOT(sl_textChanged()));
    connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        SLOT(sl_selectionChanged()));
    connect(addDirButton, SIGNAL(clicked()), SLOT(sl_addDir()));
    connect(absolutePathButton, SIGNAL(clicked()), SLOT(sl_saveToFS()));
}

FSItem * OutputFileDialog::selectedItem() const {
    QModelIndexList idxs = selectionModel->selectedIndexes();
    CHECK(!idxs.isEmpty(), NULL);

    return model->toItem(idxs.first());
}

QString OutputFileDialog::selectedPath() const {
    FSItem *item = selectedItem();
    SAFE_POINT(NULL != item, "NULL item", "");

    if (!saveDir && !item->isDir()) {
        item = item->parent();
    }

    return model->getPath(item);
}

void OutputFileDialog::sl_selectionChanged() {
    FSItem *item = selectedItem();
    SAFE_POINT(NULL != item, "NULL item", );

    if (!item->isDir()) {
        nameEdit->setText(item->name());
    }
    updateSaveButton();
}

void OutputFileDialog::sl_textChanged() {
    FSItem *item = selectedItem();
    SAFE_POINT(NULL != item, "NULL item", );

    if (!item->isDir()) {
        QModelIndexList idxs = selectionModel->selectedIndexes();
        SAFE_POINT(1 == idxs.size(), "Unselected items", );
        selectionModel->select(idxs.first().parent(), QItemSelectionModel::ClearAndSelect);
    } else {
        updateSaveButton();
    }
}

void OutputFileDialog::sl_addDir() {
    CreateDirectoryDialog d(rfs, selectedPath(), this);
    if (d.exec()) {
        QModelIndexList idxs = selectionModel->selectedIndexes();
        CHECK(!idxs.isEmpty(), );
        QModelIndex index = idxs.first();

        FSItem *item = model->toItem(index);
        SAFE_POINT(NULL != item, "NULL item", );
        if (!item->isDir()) {
            index = index.parent();
        }
        model->addDir(index, d.getResult());
    }
    updateFocus();
}

void OutputFileDialog::sl_saveToFS() {
    saveToFileSystem = true;
    reject();
}

void OutputFileDialog::updateSaveButton() {
    QPushButton *saveButton = buttonBox->button(QDialogButtonBox::Save);
    QString path = getResult();
    saveButton->setEnabled(rfs->canAdd(path, saveDir));

    nameEdit->setToolTip(path);
    QString dir = selectedPath();
    if (dir.isEmpty()) {
        dir = OUT_DIR;
    } else {
        dir += "/";
    }
    dirLabel->setText(dir);
}

QString OutputFileDialog::getResult() const {
    if (saveToFileSystem) {
        return nameEdit->text();
    }
    QString path = selectedPath();
    if (!saveDir) {
        if (!path.isEmpty()) {
            path += "/";
        }
        path += nameEdit->text();
    }
    return path;
}

bool OutputFileDialog::isSaveToFileSystem() const {
    return saveToFileSystem;
}

void OutputFileDialog::updateFocus() {
    if (saveDir) {
        treeView->setFocus();
    } else {
        nameEdit->setFocus();
    }
}

/************************************************************************/
/* CreateDirectoryDialog */
/************************************************************************/
CreateDirectoryDialog::CreateDirectoryDialog(RunFileSystem *_rfs, const QString &_parentDir, QWidget *parent)
: QDialog(parent), rfs(_rfs), parentDir(_parentDir)
{
    setupUi(this);
    if (parentDir.isEmpty()) {
        dirLabel->setText(OUT_DIR);
    } else {
        parentDir += "/";
        dirLabel->setText(parentDir);
    }
    sl_textChanged();

    connect(nameEdit, SIGNAL(textEdited(const QString &)), SLOT(sl_textChanged()));
}

void CreateDirectoryDialog::sl_textChanged() {
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    if (nameEdit->text().isEmpty()) {
        okButton->setEnabled(false);
        return;
    }
    QString path = parentDir + nameEdit->text();
    if (rfs->contains(path)) {
        okButton->setEnabled(false);
        return;
    }
    okButton->setEnabled(rfs->canAdd(path, true));
}

QString CreateDirectoryDialog::getResult() const {
    return nameEdit->text();
}

/************************************************************************/
/* RFSTreeModel */
/************************************************************************/
RFSTreeModel::RFSTreeModel(FSItem *rfsRoot, bool _saveDir, QObject *parent)
: QAbstractItemModel(parent), saveDir(_saveDir)
{
    rootItem = new FSItem("", true);
    rootItem->addChild(rfsRoot);
}

RFSTreeModel::~RFSTreeModel() {
    //delete rootItem;
}

QVariant RFSTreeModel::data(const QModelIndex &index, int role) const {
    CHECK(index.isValid(), QVariant());

    FSItem *item = toItem(index);

    if (Qt::DisplayRole == role) {
        return item->name();
    } else if (Qt::DecorationRole == role) {
        QString iconStr;
        if (item->isDir()) {
            iconStr = ":U2Designer/images/directory.png";
        } else {
            iconStr = ":U2Designer/images/file.png";
        }
        return QIcon(iconStr);
    }

    return QVariant();
}

Qt::ItemFlags RFSTreeModel::flags(const QModelIndex &index) const {
    CHECK(index.isValid(), 0);

    if (toItem(index)->isDir() || !saveDir) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return 0;
}

QVariant RFSTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    return QVariant();
}

QModelIndex RFSTreeModel::index(int row, int column, const QModelIndex &parent) const {
    CHECK(hasIndex(row, column, parent), QModelIndex());

    FSItem *parentItem = rootItem;
    if (parent.isValid()) {
        parentItem = toItem(parent);
    }

    FSItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex RFSTreeModel::parent(const QModelIndex &index) const {
    CHECK(index.isValid(), QModelIndex());

    FSItem *parentItem = toItem(index)->parent();
    CHECK(parentItem != rootItem, QModelIndex());

    return createIndex(parentItem->row(), 0, parentItem);
}

int RFSTreeModel::rowCount(const QModelIndex &parent) const {
    CHECK(0 >= parent.column(), 0);

    FSItem *parentItem = rootItem;
    if (parent.isValid()) {
        parentItem = toItem(parent);
    }

    CHECK(parentItem->isDir(), 0);
    return parentItem->children().size();
}

int RFSTreeModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

QString RFSTreeModel::getPath(FSItem *target) const {
    FSItem *root = rootItem->child(0);

    QStringList result;
    FSItem *item = target;
    while (item != root) {
        result.prepend(item->name());
        item = item->parent();
    }
    return result.join("/");
}

FSItem * RFSTreeModel::toItem(const QModelIndex &index) const {
    CHECK(index.isValid(), NULL);
    return static_cast<FSItem*>(index.internalPointer());
}

void RFSTreeModel::addDir(const QModelIndex &index, const QString &dirName) {
    FSItem *item = toItem(index);
    beginInsertRows(index, 0, item->children().size());
    item->addChild(new FSItem(dirName, true));
    endInsertRows();
}

} // U2
