/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>

#include "RemoteServiceMachine.h"
#include "RemoteServiceSettingsUI.h"
#include "RemoteTasksDialog.h"

static const int PORT_MAX = 0x7fff;

namespace U2 {


RemoteServiceSettingsUI::RemoteServiceSettingsUI() 
{
    setupUi(this);
}

RemoteServiceSettingsUI::~RemoteServiceSettingsUI() {
}

RemoteMachineSettingsPtr RemoteServiceSettingsUI::createMachine() const {
    if( !validate().isEmpty()) {
        return RemoteMachineSettingsPtr();
    }
    
    QString url = urlLineEdit->text().trimmed();

    RemoteServiceSettingsPtr s( new RemoteServiceMachineSettings(url) );

    if (guestRB->isChecked()) {
        s->setupCredentials(RemoteServiceMachineSettings::GUEST_ACCOUNT, QString(), true);
    } else {
        bool permanent = rememberEdit->isChecked();
        s->setupCredentials(userNameEdit->text().trimmed(), passwdEdit->text(), permanent);
    }
    
    return s;

}

void RemoteServiceSettingsUI::initializeWidget(const RemoteMachineSettingsPtr& settings) {
    //TODO: remove this crap
    const RemoteServiceSettingsPtr castedSettings = qSharedPointerDynamicCast<RemoteServiceMachineSettings>(settings);
    if (NULL != castedSettings) {
        urlLineEdit->setText(castedSettings->getUrl());

        if (castedSettings->usesGuestAccount()) {
            guestRB->setChecked(true);
        } else {
            existingRB->setChecked(true);
            const UserCredentials& credentials = settings->getUserCredentials();
            if (credentials.valid) {
                userNameEdit->setText(credentials.name);
                passwdEdit->setText(credentials.passwd);
                rememberEdit->setChecked(credentials.permanent);
            }
        }        
    }
}

QString RemoteServiceSettingsUI::validate() const {
    
    QString url = urlLineEdit->text();
    if( url.isEmpty() ) {
        return tr( "Url is empty!" );
    } else if (!url.startsWith("http://") && !(url.startsWith("https://"))) {
        return tr("Unknown URL schema. The url must start from http(s)://");
    }

    if (existingRB->isChecked()) {
        if( userNameEdit->text().isEmpty()) {
            return tr( "User name is not valid!" );
        }
        if( passwdEdit->text().isEmpty())  {
            return tr( "Password is empty!" );
        }
    }

    return QString();
}

void RemoteServiceSettingsUI::clearWidget() {


}

QDialog* RemoteServiceSettingsUI::createUserTasksDialog( const RemoteMachineSettingsPtr& s, QWidget* parent ) {
    //TODO: remove this crap
    const RemoteServiceSettingsPtr settings = qSharedPointerDynamicCast< RemoteServiceMachineSettings >(s);
    
    if (settings == NULL) {
        return NULL;
    }

    return new RemoteTasksDialog(settings, parent);
}



} // namespace U2
