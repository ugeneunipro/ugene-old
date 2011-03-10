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

RemoteMachineSettings * RemoteServiceSettingsUI::createMachine() const {
    if( !validate().isEmpty()) {
        return NULL;
    }
    
    QString url = urlLineEdit->text().trimmed();

    RemoteServiceMachineSettings* s = new RemoteServiceMachineSettings(url);

    if (guestRB->isChecked()) {
        s->setupCredentials(RemoteServiceMachineSettings::GUEST_ACCOUNT, QString(), true);
    } else {
        bool permanent = rememberEdit->isChecked();
        s->setupCredentials(userNameEdit->text().trimmed(), passwdEdit->text(), permanent);
    }
    
    return s;

}

void RemoteServiceSettingsUI::initializeWidget(const RemoteMachineSettings *settings) {
    const RemoteServiceMachineSettings *castedSettings = dynamic_cast<const RemoteServiceMachineSettings *>(settings);
    if (NULL != castedSettings) {
        urlLineEdit->setText(castedSettings->getUrl());

        if (castedSettings->usesGuestAccount()) {
            guestRB->setChecked(true);
        } else {
            existingRB->setChecked(true);
            UserCredentials* credentials = settings->getUserCredentials();
            if (credentials != NULL) {
                userNameEdit->setText(castedSettings->getUserName());
                passwdEdit->setText(castedSettings->getPasswd());
                rememberEdit->setChecked(credentials->permanent);
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

QDialog* RemoteServiceSettingsUI::createUserTasksDialog( const RemoteMachineSettings* s, QWidget* parent ) {
    const RemoteServiceMachineSettings* settings = dynamic_cast< const RemoteServiceMachineSettings* >(s);
    
    if (settings == NULL) {
        return NULL;
    }

    return new RemoteTasksDialog(settings, parent);
}



} // namespace U2
