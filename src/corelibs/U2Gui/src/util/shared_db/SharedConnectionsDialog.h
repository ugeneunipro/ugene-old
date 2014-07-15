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

#ifndef _U2_SHARED_CONNECTIONS_DIALOG_H_
#define _U2_SHARED_CONNECTIONS_DIALOG_H_

#include <QtGui/QDialog>

#include <U2Core/global.h>

namespace Ui {
class SharedConnectionsDialog;
}

class QListWidgetItem;
class QModelIndex;

namespace U2 {

class U2DbiRef;
class Task;

class U2GUI_EXPORT SharedConnectionsDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(SharedConnectionsDialog)
public:
    explicit SharedConnectionsDialog(QWidget *parent = 0);
    ~SharedConnectionsDialog();
    
private slots:
    void sl_selectionChanged();
    void sl_itemDoubleClicked(const QModelIndex& index);
    void sl_connectClicked();
    void sl_disconnectClicked();
    void sl_editClicked();
    void sl_addClicked();
    void sl_deleteClicked();
    void sl_connectionComplete();

private:
    void init();
    void connectSignals();
    bool askCredentials(const QString &dbUrl);

    void updateState();
    void updateButtonsState();
    void updateConnectionsState();
    void updateItemIcon(QListWidgetItem* item, bool isConnected);

    void restoreRecentConnections();
    void removeRecentConnection(const QListWidgetItem *item) const;
    void saveRecentConnection(const QListWidgetItem *item) const;
    void saveRecentConnections() const;
    void addConnectionsFromProject();
    void addPredefinedConnection();

    bool checkDatabaseAvailability(const U2DbiRef &ref, bool &initializationRequired);
    bool isConnected(QListWidgetItem *item) const;
    QStringList getDbUrls() const;
    QListWidgetItem* insertConnection(const QString& preferredName, const QString& dbUrl);
    void cancelConnection(QListWidgetItem* item);
    QString rollName(const QString& preferredName) const;
    void countConnectionsToPublicDatabase(const QString &dbUrl);

    Ui::SharedConnectionsDialog *ui;
    QMap<QListWidgetItem*, Task*> connectionTasks;

    static const QString SETTINGS_RECENT;

    static const QString PUBLIC_DATABASE_NAME;
    static const QString PUBLIC_DATABASE_URL;
    static const QString PUBLIC_DATABASE_LOGIN;
    static const QString PUBLIC_DATABASE_PASSWORD;
};

}   // namespace U2

#endif // _U2_SHARED_CONNECTIONS_DIALOG_H_
