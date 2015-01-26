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

#include <QtGui/QContextMenuEvent>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDesktopWidget>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>
#endif

#include <U2Core/AddDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2ObjectTypeUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Designer/DatasetsController.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/ProjectTreeControllerModeSettings.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>
#include <U2Gui/ProjectUtils.h>
#include <U2Gui/SharedConnectionsDialog.h>
#include <U2Gui/U2FileDialog.h>

#include <U2Lang/SharedDbUrlUtils.h>

#include "DatasetWidget.h"

namespace U2 {

URLListWidget::URLListWidget(URLListController *_ctrl)
    : QWidget(), ctrl(_ctrl), connectToDbDialog(new SharedConnectionsDialog(this)), waitingForDbToConnect(false)
{
    setupUi(this);
    popup = new OptionsPopup(this);

    reset();
    QIcon fileIcon = QIcon(QString(":U2Designer/images/add_file.png"));
    QIcon dirIcon = QIcon(QString(":U2Designer/images/add_directory.png"));
    QIcon dbIcon = QIcon(QString(":U2Designer/images/database_add.png"));
    QIcon deleteIcon = QIcon(QString(":U2Designer/images/exit.png"));
    QIcon upIcon = QIcon(QString(":U2Designer/images/up.png"));
    QIcon downIcon = QIcon(QString(":U2Designer/images/down.png"));

    addFileButton->setIcon(fileIcon);
    addDirButton->setIcon(dirIcon);
    addFromDbButton->setIcon(dbIcon);
    deleteButton->setIcon(deleteIcon);
    upButton->setIcon(upIcon);
    downButton->setIcon(downIcon);

    connect(addFileButton, SIGNAL(clicked()), SLOT(sl_addFileButton()));
    connect(addDirButton, SIGNAL(clicked()), SLOT(sl_addDirButton()));
    connect(addFromDbButton, SIGNAL(clicked()), SLOT(sl_addFromDbButton()));
    connect(downButton, SIGNAL(clicked()), SLOT(sl_downButton()));
    connect(upButton, SIGNAL(clicked()), SLOT(sl_upButton()));
    connect(deleteButton, SIGNAL(clicked()), SLOT(sl_deleteButton()));
    connect(connectToDbDialog, SIGNAL(si_connectionCompleted()), SLOT(sl_sharedDbConnected()));

    connect(itemsArea, SIGNAL(itemSelectionChanged()), SLOT(sl_itemChecked()));

    if (!readingFromDbIsSupported()) {
        addFromDbButton->hide();
    }

    QAction *deleteAction = new QAction(itemsArea);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setShortcutContext(Qt::WidgetShortcut);
    connect(deleteAction, SIGNAL(triggered()), SLOT(sl_deleteButton()));
    itemsArea->addAction(deleteAction);

    QAction *selectAction = new QAction(itemsArea);
    selectAction->setShortcut(QKeySequence::SelectAll);
    selectAction->setShortcutContext(Qt::WidgetShortcut);
    connect(selectAction, SIGNAL(triggered()), SLOT(sl_selectAll()));
    itemsArea->addAction(selectAction);

    itemsArea->installEventFilter(this);
}

void URLListWidget::addUrlItem(UrlItem *urlItem) {
    urlItem->setParent(itemsArea);
    itemsArea->addItem(urlItem);
    connect(urlItem, SIGNAL(si_dataChanged()), SLOT(sl_dataChanged()));
}

void URLListWidget::sl_addFileButton() {
    LastUsedDirHelper lod;
    QStringList files;
#if defined(Q_OS_MAC) || (QT_VERSION >= 0x050000)
    if (qgetenv("UGENE_GUI_TEST").toInt() == 1 && qgetenv("UGENE_USE_NATIVE_DIALOGS").toInt() == 0) {
        files = U2FileDialog::getOpenFileNames(NULL, tr("Select file"), lod.dir, "", 0, QFileDialog::DontUseNativeDialog);
    } else
#endif
    files = U2FileDialog::getOpenFileNames(NULL, tr("Select file"), lod.dir);
    foreach (const QString &file, files) {
        lod.url = file;
        addUrl(file);
    }
}

void URLListWidget::sl_addDirButton() {
    LastUsedDirHelper lod;
    QString dir = U2FileDialog::getExistingDirectory(NULL, tr("Select a directory"), lod.dir);
    if (!dir.isEmpty()) {
        lod.dir = dir;
        addUrl(dir);
    }
}

namespace {

ProjectTreeControllerModeSettings createProjectTreeSettings(const QSet<GObjectType> &compatibleObjTypes) {
    ProjectTreeControllerModeSettings settings;
    settings.objectTypesToShow += compatibleObjTypes;

    Project *proj = AppContext::getProject();
    SAFE_POINT(NULL != proj, "Invalid project", settings);

    foreach (Document *doc, proj->getDocuments()) {
        if (!doc->isDatabaseConnection()) {
            settings.excludeDocList << doc;
        }
    }

    return settings;
}

}

void URLListWidget::sl_sharedDbConnected() {
    SAFE_POINT(waitingForDbToConnect, "Unexpected database state", );
    waitingForDbToConnect = false;
    sl_addFromDbButton();
}

void URLListWidget::sl_addFromDbButton() {
    CHECK(!waitingForDbToConnect, );
    if (!ProjectUtils::areSharedDatabasesAvailable()) {
         if (QDialog::Accepted == connectToDbDialog->exec()) {
             waitingForDbToConnect = true;
         }
         return;
     } else {
        waitingForDbToConnect = false;
     }

    const QSet<GObjectType> compatTypes = ctrl->getCompatibleObjTypes();
    SAFE_POINT(!compatTypes.isEmpty(), "Invalid object types", );
    const ProjectTreeControllerModeSettings settings = createProjectTreeSettings(compatTypes);

    QList<Folder> folders;
    QList<GObject *> objects;
    ProjectTreeItemSelectorDialog::selectObjectsAndFolders(settings, this, folders, objects);

    foreach (const Folder &f, folders) {
        // FIXME when readers for different data types appear
        addUrl(SharedDbUrlUtils::createDbFolderUrl(f, U2ObjectTypeUtils::toDataType(*compatTypes.begin())));
    }

    foreach (GObject *obj, objects) {
        addUrl(SharedDbUrlUtils::createDbObjectUrl(obj));
    }
}

void URLListWidget::addUrl(const QString &url) {
    U2OpStatusImpl os;
    ctrl->addUrl(url, os);
    if (os.hasError()) {
        QMessageBox::critical(this, tr("Error"), os.getError());
    }
}

void URLListWidget::sl_itemChecked() {
    reset();
    if (itemsArea->selectedItems().size() > 0) {
        deleteButton->setEnabled(true);
        bool firstSelected = itemsArea->item(0)->isSelected();
        bool lastSelected = itemsArea->item(itemsArea->count() - 1)->isSelected();
        upButton->setEnabled(!firstSelected);
        downButton->setEnabled(!lastSelected);
    }
}

void URLListWidget::reset() {
    deleteButton->setEnabled(false);
    upButton->setEnabled(false);
    downButton->setEnabled(false);
    popup->hideOptions();
}

bool URLListWidget::readingFromDbIsSupported() const {
    return !ctrl->getCompatibleObjTypes().isEmpty();
}

void URLListWidget::sl_downButton() {
    CHECK(itemsArea->selectedItems().size() > 0, );

    for (int pos=itemsArea->count() - 2; pos >= 0; pos--) { // without last item
        if (itemsArea->item(pos)->isSelected()) {
            QListWidgetItem *item = itemsArea->takeItem(pos);
            itemsArea->insertItem(pos+1, item);
            item->setSelected(true);
            ctrl->replaceUrl(pos, pos+1);
        }
    }
}

void URLListWidget::sl_upButton() {
    CHECK(itemsArea->selectedItems().size() > 0, );

    for (int pos=1; pos < itemsArea->count(); pos++) { // without first item
        if (itemsArea->item(pos)->isSelected()) {
            QListWidgetItem *item = itemsArea->takeItem(pos);
            itemsArea->insertItem(pos-1, item);
            item->setSelected(true);
            ctrl->replaceUrl(pos, pos-1);
        }
    }
}

void URLListWidget::sl_deleteButton() {
    foreach (QListWidgetItem *item, itemsArea->selectedItems()) {
        int pos = itemsArea->row(item);
        ctrl->deleteUrl(pos);
        delete itemsArea->takeItem(pos);
    }
}

void URLListWidget::sl_selectAll() {
    for (int i=0; i<itemsArea->count(); i++) {
        itemsArea->item(i)->setSelected(true);
    }
}

void URLListWidget::sl_dataChanged() {
    ctrl->updateUrl(dynamic_cast<UrlItem*>(sender()));
}

bool URLListWidget::eventFilter(QObject *obj, QEvent *event) {
    CHECK(itemsArea == obj, false);
    if (event->type() == QEvent::ContextMenu) {
        CHECK(1 == itemsArea->selectedItems().size(), false);

        QContextMenuEvent *e = static_cast<QContextMenuEvent *>(event);
        QListWidgetItem *item = itemsArea->itemAt(e->pos());
        CHECK(NULL != item, false);
        CHECK(item->isSelected(), false);

        UrlItem *urlItem = static_cast<UrlItem*>(item);
        CHECK(NULL != urlItem, false);

        QWidget *options = urlItem->getOptionsWidget();
        if (NULL != options) {
            popup->showOptions(options, itemsArea->mapToGlobal(e->pos()));
        }
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

/************************************************************************/
/* OptionsPopup */
/************************************************************************/
OptionsPopup::OptionsPopup(QWidget *parent)
: QFrame(parent)
{
    l = new QVBoxLayout(this);
    l->setMargin(0);
    setWindowFlags(Qt::Popup);
    setFrameShape(QFrame::StyledPanel);
}

void OptionsPopup::closeEvent(QCloseEvent *event) {
    removeOptions();
    QWidget::closeEvent(event);
}

void OptionsPopup::showOptions(QWidget *options, const QPoint &p) {
    l->insertWidget(0, options);
    move(p);
    show();

    // if the popup is not fit in the screen, then move it
    int maxWidth = QApplication::desktop()->width();
    int maxHeight = QApplication::desktop()->height();
    QPoint rightBottom = pos() + QPoint(width(), height());
    if (rightBottom.x() > maxWidth) {
        move(x() - (rightBottom.x() - maxWidth), y());
    }
    if (rightBottom.y() > maxHeight) {
        move(x(), y() - (rightBottom.y() - maxHeight));
    }
}

void OptionsPopup::removeOptions() {
    QLayoutItem *child;
    while (NULL != (child = l->takeAt(0))) {
        child->widget()->setParent(NULL);
        delete child;
    }
}

void OptionsPopup::hideOptions() {
    removeOptions();
    hide();
}

} // U2
