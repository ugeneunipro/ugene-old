#include "GTUtilsDialog.h"
#include "api/GTFileDialog.h"

namespace U2 {

class EditConnectionDialogFiller : public Filler {
public:
    class Parameters {
        public:
        Parameters():
            connectionName(""),
            host(""),
            port(""),
            database(""),
            login(""),
            password(""),
            rememberMe(false),
            accept(true) {}

        QString connectionName;
        QString host;
        QString port;
        QString database;
        QString login;
        QString password;
        bool rememberMe;
        bool accept;
    };


    enum ConnectionType {FROM_SETTINGS, MANUAL};
    EditConnectionDialogFiller(U2OpStatus &os, const Parameters &parameters, ConnectionType type);
    void run();

private:
    Parameters parameters;
};

class AuthenticationDialogFiller : public Filler {
public:
    AuthenticationDialogFiller(U2OpStatus &os, const QString &login, const QString &password);
    void run();

private:
    QString login;
    QString password;
};

} // U2
