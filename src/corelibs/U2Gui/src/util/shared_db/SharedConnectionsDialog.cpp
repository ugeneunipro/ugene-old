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

#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QMessageBox>

#include <U2Core/AddDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/CredentialsStorage.h>
#include <U2Core/ConnectSharedDatabaseTask.h>
#include <U2Core/Counter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Settings.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/MysqlDbiUtils.h>

#include <U2Gui/AuthenticationDialog.h>

#include "EditConnectionDialog.h"
#include "SharedConnectionsDialog.h"
#include "ui/ui_SharedConnectionsDialog.h"


static const char * NON_INITED_DB_MB_TITLE = "UGENE Shared Database Setup";
static const char * NON_INITED_DB_MB_TEXT = "UGENE has detected that the database "
    "you are connecting to is not initialized. Do you want to set it up now for use with UGENE?";

static const char * UNABLE_TO_CONNECT_TITLE = "Could Not Connect";
static const char * UNABLE_TO_CONNECT_TEXT = "The database has been set up "
    "for a more recent version of UGENE, this means that this version of UGENE is not compatible "
    "with the database and will not connect to it. Upgrade UGENE to at least %1 version "
    "to make use of the database";

namespace U2 {

const QString SharedConnectionsDialog::SETTINGS_RECENT = "/shared_database/recent_connections/";
const QString SharedConnectionsDialog::PUBLIC_DATABASE_NAME = tr("UGENE public database");
const QString SharedConnectionsDialog::PUBLIC_DATABASE_URL = U2DbiUtils::createDbiUrl("5.9.139.103", 3306, "public_ugene");
const QString SharedConnectionsDialog::PUBLIC_DATABASE_LOGIN = "public";
const QString SharedConnectionsDialog::PUBLIC_DATABASE_PASSWORD = "public";

SharedConnectionsDialog::SharedConnectionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SharedConnectionsDialog)
{
    ui->setupUi(this);

    init();
    connectSignals();
    updateState();
}

SharedConnectionsDialog::~SharedConnectionsDialog() {
    delete ui;
}

void SharedConnectionsDialog::sl_selectionChanged() {
    updateState();
}

void SharedConnectionsDialog::sl_itemDoubleClicked(const QModelIndex& index) {
    ui->lwConnections->setCurrentIndex(index);
    if (!isConnected(ui->lwConnections->currentItem())) {
        sl_connectClicked();
    }
}

void SharedConnectionsDialog::sl_connectClicked() {
    QListWidgetItem *selectedItem = ui->lwConnections->currentItem();
    SAFE_POINT(NULL != selectedItem, "Invalid list item detected", );
    const QString dbUrl = selectedItem->data(Qt::UserRole).toString();
    const QString connectionName = selectedItem->data(Qt::DisplayRole).toString();

    countConnectionsToPublicDatabase(dbUrl);

    // TODO: don't forget to change this when new DB providers will be introduced
    const U2DbiRef dbiRef(MYSQL_DBI_ID, dbUrl);

    if (!AppContext::getCredentialsStorage()->contains(dbUrl)) {
        if (!askCredentials(dbUrl)) {
            return;
        }
    }

    bool initializeDb = false;
    if (!checkDatabaseAvailability(dbiRef, initializeDb)) {
        return;
    }
    
    Task *dbLoadTask = new AddDocumentTask(new ConnectSharedDatabaseTask(dbiRef, connectionName, initializeDb));
    connect(dbLoadTask, SIGNAL(si_stateChanged()), SLOT(sl_connectionComplete()));
    connectionTasks.insert(ui->lwConnections->currentItem(), dbLoadTask);
    AppContext::getTaskScheduler()->registerTopLevelTask(dbLoadTask);

    accept();
}

void SharedConnectionsDialog::sl_disconnectClicked() {
    const QString dbUrl = ui->lwConnections->currentItem()->data(Qt::UserRole).toString();

    cancelConnection(ui->lwConnections->currentItem());

    Document* doc = AppContext::getProject()->findDocumentByURL(dbUrl);
    if (NULL != doc) {
        AppContext::getProject()->removeDocument(doc);
    }

    updateState();
}

void SharedConnectionsDialog::sl_editClicked() {
    const U2DbiId dbUrl = ui->lwConnections->currentItem()->data(Qt::UserRole).toString();
    const QString name = ui->lwConnections->currentItem()->text();

    EditConnectionDialog editDialog(this, dbUrl, name);
    if (QDialog::Accepted == editDialog.exec()) {
        QListWidgetItem* item = ui->lwConnections->currentItem();
        if (name != editDialog.getName()) {
            removeRecentConnection(item);
        }

        item->setText(editDialog.getName());
        item->setData(Qt::UserRole, editDialog.getDbUrl());

        saveRecentConnection(item);
        updateState();
    }
}

void SharedConnectionsDialog::sl_addClicked() {
    EditConnectionDialog editDialog(this);

    if (QDialog::Accepted == editDialog.exec()) {
        QListWidgetItem* item = insertConnection(editDialog.getName(), editDialog.getDbUrl());
        CHECK(NULL != item, );
        ui->lwConnections->setCurrentItem(item);
        saveRecentConnection(item);
        updateState();
    }
}

void SharedConnectionsDialog::sl_deleteClicked() {
    if (ui->pbDisconnect->isEnabled()) {
        sl_disconnectClicked();
    }

    const QString dbUrl = ui->lwConnections->currentItem()->data(Qt::UserRole).toString();
    AppContext::getCredentialsStorage()->removeEntry(dbUrl);

    int index = ui->lwConnections->currentRow();
    QListWidgetItem* item = ui->lwConnections->takeItem(index);
    ui->lwConnections->setCurrentRow(qMin<int>(index, ui->lwConnections->count() - 1));

    cancelConnection(item);

    removeRecentConnection(item);
    delete item;
    updateState();
}

void SharedConnectionsDialog::sl_connectionComplete() {
    AddDocumentTask* task = qobject_cast<AddDocumentTask*>(sender());
    if (!task->isFinished()) {
        return;
    }

    connectionTasks.remove(connectionTasks.key(task));

    updateState();
}

void SharedConnectionsDialog::init() {
    restoreRecentConnections();
    addPredefinedConnection();
    addConnectionsFromProject();
    saveRecentConnections();
}

void SharedConnectionsDialog::connectSignals() {
    connect(ui->lwConnections,  SIGNAL(itemSelectionChanged()),     SLOT(sl_selectionChanged()));
    connect(ui->lwConnections,  SIGNAL(doubleClicked(QModelIndex)), SLOT(sl_itemDoubleClicked(QModelIndex)));

    connect(ui->pbConnect,      SIGNAL(clicked()),      SLOT(sl_connectClicked()));
    connect(ui->pbDisconnect,   SIGNAL(clicked()),      SLOT(sl_disconnectClicked()));
    connect(ui->pbEdit,         SIGNAL(clicked()),      SLOT(sl_editClicked()));
    connect(ui->pbAdd,          SIGNAL(clicked()),      SLOT(sl_addClicked()));
    connect(ui->pbDelete,       SIGNAL(clicked()),      SLOT(sl_deleteClicked()));
}

void SharedConnectionsDialog::updateState() {
    updateButtonsState();
    updateConnectionsState();
}

void SharedConnectionsDialog::updateButtonsState() {
    QListWidgetItem* currentItem = ui->lwConnections->currentItem();
    const bool isSomethingSelected = (NULL != currentItem);
    const bool isCurrentConnected = isConnected(currentItem);

    ui->pbDelete->setEnabled(isSomethingSelected);
    ui->pbEdit->setEnabled(isSomethingSelected && !isCurrentConnected);
    ui->pbConnect->setEnabled(isSomethingSelected && !isCurrentConnected);
    ui->pbDisconnect->setEnabled(isSomethingSelected && isCurrentConnected);
}

void SharedConnectionsDialog::updateConnectionsState() {
    for (int i = 0; i < ui->lwConnections->count(); i++) {
        QListWidgetItem* item = ui->lwConnections->item(i);
        updateItemIcon(item, isConnected(item));
    }
}

void SharedConnectionsDialog::updateItemIcon(QListWidgetItem* item, bool isConnected) {
    QPixmap px(":/core/images/db/database_lightning.png");
    if (!isConnected) {
        px.fill(Qt::transparent);
    }
    item->setIcon(QIcon(px));
}

bool SharedConnectionsDialog::askCredentials(const QString& dbUrl) {
    AuthenticationDialog authenticationDialog(tr("Connect to the database: '%1'").arg(dbUrl), this);
    if (QDialog::Accepted == authenticationDialog.exec()) {
        Credentials credentials(authenticationDialog.getLogin(), authenticationDialog.getPassword());
        AppContext::getCredentialsStorage()->addEntry(dbUrl, credentials, authenticationDialog.isRemembered());
        return true;
    } else {
        return false;
    }
}

void SharedConnectionsDialog::restoreRecentConnections() {
    const QStringList recentList = AppContext::getSettings()->getAllKeys(SETTINGS_RECENT);
    foreach (const QString& recent, recentList) {
        const QString data = AppContext::getSettings()->getValue(SETTINGS_RECENT + recent).toString();
        insertConnection(recent, data);
    }
}

void SharedConnectionsDialog::removeRecentConnection(const QListWidgetItem* item) const {
    AppContext::getSettings()->remove(SETTINGS_RECENT + item->text());
}

void SharedConnectionsDialog::saveRecentConnection(const QListWidgetItem* item) const {
    AppContext::getSettings()->setValue(SETTINGS_RECENT + item->text(), item->data(Qt::UserRole));
}

void SharedConnectionsDialog::saveRecentConnections() const {
    AppContext::getSettings()->cleanSection(SETTINGS_RECENT);

    for (int i = 0; i < ui->lwConnections->count(); i++) {
        const QListWidgetItem* item = ui->lwConnections->item(i);
        saveRecentConnection(item);
    }
}

void SharedConnectionsDialog::addConnectionsFromProject() {
    if (NULL == AppContext::getProject()) {
        return;
    }

    const QStringList urls = getDbUrls();
    const QList<Document*> docList = AppContext::getProject()->getDocuments();
    foreach (const Document* doc, docList) {
        if (GUrl_Network == doc->getURL().getType() && !urls.contains(doc->getURLString())) {
            insertConnection(doc->getName(), doc->getURLString());
        }
    }
}

void SharedConnectionsDialog::addPredefinedConnection() {
    if (!getDbUrls().contains(PUBLIC_DATABASE_URL)) {
        insertConnection(PUBLIC_DATABASE_NAME, PUBLIC_DATABASE_URL);
        Credentials credentials(PUBLIC_DATABASE_LOGIN, PUBLIC_DATABASE_PASSWORD);
        AppContext::getCredentialsStorage()->addEntry(PUBLIC_DATABASE_URL, credentials, true);
    }
}

bool SharedConnectionsDialog::checkDatabaseAvailability(const U2DbiRef &ref, bool &initializationRequired) {
    U2OpStatusImpl os;
    const bool dbInitialized = MysqlDbiUtils::isDbInitialized(ref, os);
    if (os.isCoR()) {
        QMessageBox::critical(this,
                              tr("Connection Error"),
                              tr("Unable to connect to the database:\n"
                                 "check the connection settings"));
        return false;
    }

    if (!dbInitialized) {
        int userInput = QMessageBox::question(this, tr(NON_INITED_DB_MB_TITLE),
            tr(NON_INITED_DB_MB_TEXT), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        QMessageBox::StandardButton answer = static_cast<QMessageBox::StandardButton>(userInput);
        switch (answer) {
        case QMessageBox::No :
            initializationRequired = false;
            return false;
        case QMessageBox::Yes :
            initializationRequired = true;
            return true;
        default:
            FAIL("Unexpected user answer detected!", false);
        }
    }

    QString minRequiredVersion;
    const bool isAppVersionSufficient = MysqlDbiUtils::dbSatisfiesAppVersion(ref,
        minRequiredVersion, os);
    SAFE_POINT_OP(os, false);
    if (!isAppVersionSufficient) {
        QMessageBox::critical(this, tr(UNABLE_TO_CONNECT_TITLE), tr(UNABLE_TO_CONNECT_TEXT).arg(minRequiredVersion));
        return false;
    }
    return true;
}

bool SharedConnectionsDialog::isConnected(QListWidgetItem* item) const {
    CHECK(NULL != item, false);
    CHECK(NULL != AppContext::getProject(), false);

    bool connectionIsInProcess = connectionTasks.contains(item);

    Document* connectionDoc = AppContext::getProject()->findDocumentByURL(GUrl(item->data(Qt::UserRole).toString(), GUrl_Network));

    return ((NULL != connectionDoc) && (connectionDoc->isLoaded()))
            || connectionIsInProcess;
}

QStringList SharedConnectionsDialog::getDbUrls() const{
    QStringList result;

    for (int i = 0; i < ui->lwConnections->count(); i++) {
        result << ui->lwConnections->item(i)->data(Qt::UserRole).toString();
    }

    return result;
}

QListWidgetItem* SharedConnectionsDialog::insertConnection(const QString& preferredName, const QString& dbUrl) {
    CHECK(!getDbUrls().contains(dbUrl), NULL);
    const QString name = rollName(preferredName);

    QListWidgetItem* item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, dbUrl);
    ui->lwConnections->addItem(item);
    return item;
}

void SharedConnectionsDialog::cancelConnection(QListWidgetItem* item) {
    Task* connectionTask = connectionTasks.value(item, NULL);
    if (NULL != connectionTask) {
        connectionTask->cancel();
        connectionTasks.remove(item);
    }
}

QString SharedConnectionsDialog::rollName(const QString &preferredName) const {
    QString name = preferredName;
    int i = 0;
    while (!ui->lwConnections->findItems(name, Qt::MatchExactly).isEmpty()) {
        i++;
        name = preferredName + " (" + QString::number(i) + ")";
    }
    return name;
}

void SharedConnectionsDialog::countConnectionsToPublicDatabase(const QString &dbUrl) {
    if (PUBLIC_DATABASE_URL == dbUrl) {
        GCOUNTER(cvar, tvar, "Connections to public database");
    }
}

}   // namespace U2
