#include "utils/GTUtilsDialog.h"
#include <base_dialogs/GTFileDialog.h>

namespace U2 {
using namespace HI;

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
            accept(true),
            checkDefaults(false) {}

        QString connectionName;
        QString host;
        QString port;
        QString database;
        QString login;
        QString password;
        bool rememberMe;
        bool accept;
        bool checkDefaults;
    };


    enum ConnectionType {FROM_SETTINGS, MANUAL};
    EditConnectionDialogFiller(U2OpStatus &os, const Parameters &parameters, ConnectionType type);
    EditConnectionDialogFiller(U2OpStatus &os, CustomScenario *scenario);
    void commonScenario();

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
