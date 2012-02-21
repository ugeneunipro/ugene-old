#ifndef _U2_GUI_TEST_H_
#define _U2_GUI_TEST_H_

#include <U2Core/global.h>
#include <U2Core/U2OpStatus.h>

namespace U2 {

class U2TEST_EXPORT GUITest: public QObject{
    Q_OBJECT
public:
    GUITest(const QString &_name = ""): QObject(NULL), name(_name) {} //parent must bu NULL to push object to main thread

    GUITest(GUITest* t): QObject(NULL), name(t->getName()) {}

    void launch(U2OpStatus &os);

    QString getName() const { return name; }
    void setName(const QString &n) { name = n; }

    static const QString testDir;
protected:
    virtual void execute(U2OpStatus &os) = 0;

private:
    QString name;
};

typedef QList<GUITest*> GUITests;

} //U2

#endif
