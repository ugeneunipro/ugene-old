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

#include <QtGui/QMainWindow>

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>

#include "CredentialsAskerGui.h"
#include "AuthenticationDialog.h"

namespace U2 {

CredentialsAskerGui::CredentialsAskerGui()
{
}

bool CredentialsAskerGui::ask(const QString& resourceId) {
    QWidget* mainWindow = qobject_cast<QWidget*>(AppContext::getMainWindow()->getQMainWindow());
    AuthenticationDialog authDialog(QObject::tr("Connect to the ") + resourceId, mainWindow);
    Credentials result;

    if (QDialog::Accepted != authDialog.exec()) {
        return false;
    }

    result.login = authDialog.getLogin();
    result.password = authDialog.getPassword();

    saveCredentials(resourceId, result, authDialog.isRemembered());

    return true;
}

void CredentialsAskerGui::saveCredentials(const QString& resourceId, const Credentials& credentials, bool remember) const {
    CredentialsStorage* storage = AppContext::getCredentialsStorage();
    CHECK(NULL != storage, );

    storage->addEntry(resourceId, credentials, remember);
}

}   // namespace U2
