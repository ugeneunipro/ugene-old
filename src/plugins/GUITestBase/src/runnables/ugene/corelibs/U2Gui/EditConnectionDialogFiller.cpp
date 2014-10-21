#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QGroupBox>
#include <QtGui/QComboBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#endif

#include "GTDatabaseConfig.h"
#include "api/GTAction.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTRadioButton.h"

#include "EditConnectionDialogFiller.h"

namespace U2 {

EditConnectionDialogFiller::EditConnectionDialogFiller(U2OpStatus &os, const Parameters &parameters, ConnectionType type)
: Filler(os, "EditConnectionDialog"), parameters(parameters)
{
    if (FROM_SETTINGS == type) {
        this->parameters.host = GTDatabaseConfig::host();
        this->parameters.port = QString::number(GTDatabaseConfig::port());
        this->parameters.database = GTDatabaseConfig::database();
        this->parameters.login = GTDatabaseConfig::login();
        this->parameters.password = GTDatabaseConfig::password();
    }
}

#define GT_CLASS_NAME "GTUtilsDialog::EditConnectionDialogFiller"
#define GT_METHOD_NAME "run"

void EditConnectionDialogFiller::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");
    QLineEdit* leName = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leName", dialog));
    GT_CHECK(leName, "leName is NULL");
    QLineEdit* leHost = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leHost", dialog));
    GT_CHECK(leHost, "leHost is NULL");
    QLineEdit* lePort = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "lePort", dialog));
    GT_CHECK(lePort, "lePort is NULL");
    QLineEdit* leDatabase = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leDatabase", dialog));
    GT_CHECK(leDatabase, "leDatabase is NULL");
    QLineEdit* leLogin = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leLogin", dialog));
    GT_CHECK(leLogin, "leLogin is NULL");
    QLineEdit* lePassword = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "lePassword", dialog));
    GT_CHECK(lePassword, "lePassword is NULL");
    QCheckBox* cbRemember = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "cbRemember", dialog));
    GT_CHECK(cbRemember, "cbRemember is NULL");

    if (parameters.checkDefaults) {
        GT_CHECK(lePort->text() == "3306", "Wrong default port");
    } else {
        GTLineEdit::setText(os, leName, parameters.connectionName);
        GTLineEdit::setText(os, leHost, parameters.host);
        GTLineEdit::setText(os, lePort, parameters.port);
        GTLineEdit::setText(os, leDatabase, parameters.database);
        GTLineEdit::setText(os, leLogin, parameters.login);
        GTLineEdit::setText(os, lePassword, parameters.password);
        GTCheckBox::setChecked(os, cbRemember, parameters.rememberMe);
    }

    QString buttonName = parameters.accept ? "OK" : "Cancel";
    GTWidget::click(os, GTWidget::findButtonByText(os, buttonName));
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

AuthenticationDialogFiller::AuthenticationDialogFiller(U2OpStatus &os, const QString &login, const QString &password)
: Filler(os, "AuthenticationDialog"), login(login), password(password)
{

}

#define GT_CLASS_NAME "GTUtilsDialog::AuthenticationDialogFiller"
#define GT_METHOD_NAME "run"

void AuthenticationDialogFiller::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit* leLogin = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leLogin", dialog));
    GT_CHECK(leLogin, "leLogin is NULL");
    if (leLogin->isEnabled()) {
        GTLineEdit::setText(os, leLogin, login);
    }

    QLineEdit* lePassword = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "lePassword", dialog));
    GT_CHECK(lePassword, "lePassword is NULL");
    GTLineEdit::setText(os, lePassword, password);

    QCheckBox* cbRemember = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "cbRemember", dialog));
    GT_CHECK(cbRemember, "cbRemember is NULL");
    GTCheckBox::setChecked(os, cbRemember, false);

    GTWidget::click(os, GTWidget::findButtonByText(os, "OK", dialog));
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
