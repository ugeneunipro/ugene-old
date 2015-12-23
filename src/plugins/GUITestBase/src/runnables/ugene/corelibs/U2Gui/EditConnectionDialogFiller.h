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
    EditConnectionDialogFiller(HI::GUITestOpStatus &os, const Parameters &parameters, ConnectionType type);
    EditConnectionDialogFiller(HI::GUITestOpStatus &os, CustomScenario *scenario);
    void commonScenario();

private:
    Parameters parameters;
};

class AuthenticationDialogFiller : public Filler {
public:
    AuthenticationDialogFiller(HI::GUITestOpStatus &os, const QString &login, const QString &password);
    void commonScenario();

private:
    QString login;
    QString password;
};

} // U2
