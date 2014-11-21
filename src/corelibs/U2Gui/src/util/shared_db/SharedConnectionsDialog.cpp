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
#include <QtGui/QListWidgetItem>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QMessageBox>
#endif

#include <U2Core/AddDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/PasswordStorage.h>
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
#include <U2Gui/HelpButton.h>

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

static const char * CONNECTION_DUPLICATE_TITLE = "Connection Duplicate Detected";
static const char * CONNECTION_DUPLICATE_TEXT = "You already have a connection to the database that you have specified. "
    "Existing connection name is \"%1\"";

namespace U2 {

const QString SharedConnectionsDialog::SETTINGS_RECENT = "/shared_database/recent_connections/";

SharedConnectionsDialog::SharedConnectionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SharedConnectionsDialog)
{
    ui->setupUi(this);
    new HelpButton(this, ui->buttonBox, "8093779");

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
    const QString connectionName = selectedItem->data(Qt::DisplayRole).toString();
    QString fullDbiUrl = getCurrentFullDbiUrl();

    countConnectionsToPublicDatabase(fullDbiUrl);

    if (!AppContext::getPasswordStorage()->contains(fullDbiUrl) && !askCredentials(fullDbiUrl)) {
        return;
    }

    // TODO: don't forget to change this when new DB providers will be introduced
    const U2DbiRef dbiRef(MYSQL_DBI_ID, fullDbiUrl);

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
    const QString dbiUrl = ui->lwConnections->currentItem()->data(UrlRole).toString();
    const QString userName = ui->lwConnections->currentItem()->data(LoginRole).toString();
    const QString fullDbiUrl = U2DbiUtils::createFullDbiUrl(userName, dbiUrl);

    cancelConnection(ui->lwConnections->currentItem());

    Document* doc = AppContext::getProject()->findDocumentByURL(fullDbiUrl);
    if (NULL != doc) {
        AppContext::getProject()->removeDocument(doc);
    }

    updateState();
}

void SharedConnectionsDialog::sl_editClicked() {
    const QString dbiUrl = ui->lwConnections->currentItem()->data(UrlRole).toString();
    const QString userName = ui->lwConnections->currentItem()->data(LoginRole).toString();
    const QString connectionName = ui->lwConnections->currentItem()->text();

    EditConnectionDialog editDialog(this, dbiUrl, userName, connectionName);
    editDialog.setReadOnly(U2DbiUtils::PUBLIC_DATABASE_URL == U2DbiUtils::createFullDbiUrl(userName, dbiUrl));

    if (QDialog::Accepted == editDialog.exec()) {
        QListWidgetItem* item = ui->lwConnections->currentItem();
        const QString login = editDialog.getUserName();

        QString existingConnectionName;
        if (alreadyExists(dbiUrl, login, existingConnectionName) && existingConnectionName != item->data(Qt::DisplayRole).toString()) {
            QMessageBox::information(this, tr(CONNECTION_DUPLICATE_TITLE), tr(CONNECTION_DUPLICATE_TEXT).arg(existingConnectionName));
        }

        if (connectionName != editDialog.getName()) {
            removeRecentConnection(item);
        }

        item->setText(editDialog.getName());
        item->setData(UrlRole, editDialog.getShortDbiUrl());
        item->setData(LoginRole, login);

        saveRecentConnection(item);
        updateState();
    }
}

void SharedConnectionsDialog::sl_addClicked() {
    EditConnectionDialog editDialog(this);

    if (QDialog::Accepted == editDialog.exec()) {
        QListWidgetItem* item = insertConnection(editDialog.getName(), editDialog.getShortDbiUrl(), editDialog.getUserName());
        CHECK(NULL != item, );
        ui->lwConnections->setCurrentItem(item);
        saveRecentConnection(item);
        updateState();
    }
}

void SharedConnectionsDialog::sl_deleteClicked() {
    AppContext::getPasswordStorage()->setRemembered(getCurrentFullDbiUrl(), false);

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
    emit si_connectionCompleted();
}

void SharedConnectionsDialog::init() {
    restoreRecentConnections();
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

bool SharedConnectionsDialog::askCredentials(QString &fullDbiUrl) {
    QString userName;
    const QString shortDbiUrl = U2DbiUtils::full2shortDbiUrl(fullDbiUrl, userName);

    AuthenticationDialog authenticationDialog(tr("Connect to the database %1").arg(shortDbiUrl), this);
    authenticationDialog.setLogin(userName);

    if (QDialog::Accepted == authenticationDialog.exec()) {
        fullDbiUrl = U2DbiUtils::createFullDbiUrl(authenticationDialog.getLogin(), shortDbiUrl);
        AppContext::getPasswordStorage()->addEntry(fullDbiUrl, authenticationDialog.getPassword(), authenticationDialog.isRemembered());
        return true;
    } else {
        return false;
    }
}

void SharedConnectionsDialog::restoreRecentConnections() {
    const QStringList recentList = AppContext::getSettings()->getAllKeys(SETTINGS_RECENT);
    foreach (const QString& recent, recentList) {
        const QString fullDbiUrl = AppContext::getSettings()->getValue(SETTINGS_RECENT + recent).toString();
        insertConnection(recent, fullDbiUrl);
    }
}

void SharedConnectionsDialog::removeRecentConnection(const QListWidgetItem* item) const {
    AppContext::getSettings()->remove(SETTINGS_RECENT + item->text());
}

void SharedConnectionsDialog::saveRecentConnection(const QListWidgetItem* item) const {
    AppContext::getSettings()->setValue(SETTINGS_RECENT + item->text(), getFullDbiUrl(item));
}

void SharedConnectionsDialog::saveRecentConnections() const {
    AppContext::getSettings()->cleanSection(SETTINGS_RECENT);

    for (int i = 0; i < ui->lwConnections->count(); i++) {
        const QListWidgetItem* item = ui->lwConnections->item(i);
        saveRecentConnection(item);
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
        coreLog.details(tr("Can't connect to the shared database: ") + os.getError());
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
    const bool isAppVersionSufficient = MysqlDbiUtils::dbSatisfiesAppVersion(ref, minRequiredVersion, os);
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

    Document* connectionDoc = AppContext::getProject()->findDocumentByURL(GUrl(getFullDbiUrl(item), GUrl_Network));

    return ((NULL != connectionDoc) && (connectionDoc->isLoaded())) || connectionIsInProcess;
}

bool SharedConnectionsDialog::alreadyExists(const QString &dbiUrl, const QString &userName, QString &existingName) const {
    for (int i = 0; i < ui->lwConnections->count(); i++) {
        if (dbiUrl == ui->lwConnections->item(i)->data(UrlRole).toString()
         && userName == ui->lwConnections->item(i)->data(LoginRole).toString())
        {
            existingName = ui->lwConnections->item(i)->data(Qt::DisplayRole).toString();
            return true;
        }
    }
    return false;
}

QListWidgetItem *SharedConnectionsDialog::insertConnection(const QString &preferredName, const QString &fullDbiUrl) {
    QString userName;
    const QString shortDbiUrl = U2DbiUtils::full2shortDbiUrl(fullDbiUrl, userName);
    return insertConnection(preferredName, shortDbiUrl, userName);
}

QListWidgetItem* SharedConnectionsDialog::insertConnection(const QString& preferredName, const QString& dbiUrl, const QString &userName) {
    QString existingConnectionName;
    if (alreadyExists(dbiUrl, userName, existingConnectionName)) {
        QMessageBox::information(this, tr(CONNECTION_DUPLICATE_TITLE), tr(CONNECTION_DUPLICATE_TEXT).arg(existingConnectionName));
    }

    const QString name = rollName(preferredName);

    QListWidgetItem* item = new QListWidgetItem(name);
    item->setData(UrlRole, dbiUrl);
    item->setData(LoginRole, userName);
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

void SharedConnectionsDialog::countConnectionsToPublicDatabase(const QString &dbiUrl) {
    if (U2DbiUtils::PUBLIC_DATABASE_URL == dbiUrl) {
        GCOUNTER(cvar, tvar, "Connections to public database");
    }
}

QString SharedConnectionsDialog::getCurrentFullDbiUrl() const {
    return getFullDbiUrl(ui->lwConnections->currentItem());
}

QString SharedConnectionsDialog::getFullDbiUrl(const QListWidgetItem *item) const {
    return U2DbiUtils::createFullDbiUrl(item->data(LoginRole).toString(), item->data(UrlRole).toString());
}

}   // namespace U2
