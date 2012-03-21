#ifndef _U2_GUI_TEST_H_
#define _U2_GUI_TEST_H_

#include <U2Core/global.h>
#include <QtCore/QTimer>
#include <U2Core/Task.h>

namespace U2 {

class U2TEST_EXPORT GUITest: public QObject {
    Q_OBJECT
public:
    GUITest(const QString &_name = "") : name(_name) {}

    QString getName() const { return name; }
    void setName(const QString &n) { name = n; }

    static const QString testDir;
    static const QString dataDir;

    virtual void run(U2OpStatus &os) = 0;

private:
    QString name;
};

typedef QList<GUITest*> GUITests;

} //U2

#endif
