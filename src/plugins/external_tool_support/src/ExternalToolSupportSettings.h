#ifndef _U2_EXTERNAL_TOOL_SUPPORT_SETTINGS_H_
#define _U2_EXTERNAL_TOOL_SUPPORT_SETTINGS_H_

#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtGui/QFont>

#include <U2Core/global.h>

namespace U2 {
class Watcher : public QObject {
    Q_OBJECT
signals:
    void changed();
    friend class ExternalToolSupportSettings;
};

#define SETTINGS QString("ExternalToolSupport/")

class ExternalToolSupportSettings {
public:
    static int getNumberExternalTools();
    static void setNumberExternalTools(int value);

    static bool getExternalTools();
    static void setExternalTools();

    static void checkTemporaryDir();

    static Watcher * const watcher;
private:
    static int prevNumberExternalTools;
};

}//namespace

#endif
