#ifndef _U2_GUI_TEST_H_
#define _U2_GUI_TEST_H_

#include <U2Core/global.h>
#include <QtCore/QTimer>
#include <U2Core/Task.h>

namespace U2 {

class U2TEST_EXPORT GUITest: public QObject {
    Q_OBJECT
public:
    GUITest(const QString &_name = ""): QObject(NULL), name(_name) {} //parent must bu NULL to push object to main thread

    QString getName() const { return name; }
    void setName(const QString &n) { name = n; }

    static const QString testDir;
    static const QString dataDir;

    virtual void run(U2OpStatus &os);

protected:
    virtual void execute(U2OpStatus &os) = 0;
    void testFinished();

signals:
    void finished(GUITest*);

private:
    QString name;
};

typedef QList<GUITest*> GUITests;


class U2TEST_EXPORT GUIMultiTest: public GUITest {
    Q_OBJECT
friend class GUITestService;

public:
    enum State {IDLE, RUNNING, WAITING, FINISHED} state;

    GUIMultiTest(const QString &name = "") : GUITest(name), state(IDLE), waiting(0) {}
    virtual void run(U2OpStatus &os);

protected:
    virtual void execute(U2OpStatus &){}
    void add(GUITest *t);

    TaskStateInfo os;

protected slots:
    void sl_update();
    void sl_subTestFinished(GUITest*);

private:
    GUITest* getNextTest();

    void runNextTest();
    void allTestsFinished();

    void setWaiting(int ms);
    void updateWaiting();

    void newState(State newState, State prevState);

    static const int timerInterval = 100;
    int waiting;

    QTimer timer;
    GUITests guiTests;
};

#define GENERATE_GUI_ACTION(ClassName, methodName) \
class ClassName : public GUITest { \
protected: \
    virtual void execute(U2OpStatus &os) { \
    methodName(os); \
} \
} \

#define GENERATE_GUI_ACTION_1(ClassName, methodName) \
class ClassName : public GUITest { \
public: \
    ClassName(const QString& _p1) : p1(_p1){} \
protected: \
    virtual void execute(U2OpStatus &os) { \
    methodName(os, p1); \
} \
    QString p1; \
} \

#define GENERATE_GUI_ACTION_2(ClassName, methodName) \
class ClassName : public GUITest { \
public: \
    ClassName(const QString& _p1, const QString& _p2 = QString()) : p1(_p1), p2(_p2){} \
protected: \
    virtual void execute(U2OpStatus &os) { \
    methodName(os, p1, p2); \
} \
    QString p1, p2; \
} \


} //U2

#endif
