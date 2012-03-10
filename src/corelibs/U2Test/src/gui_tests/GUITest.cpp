#include "GUITest.h"

namespace U2 {

const QString GUITest::testDir = "../../test/";
const QString GUITest::dataDir = "../../data/";

void GUITest::run(U2OpStatus &os) {
    testFinished();
    execute(os);
}

void GUITest::testFinished() {
    emit finished(this);
}

void GUIMultiTest::run(U2OpStatus &) {

    connect(&timer, SIGNAL(timeout()), this, SLOT(sl_update()), Qt::QueuedConnection);
    timer.start(timerInterval);
}

void GUIMultiTest::add(GUITest *t) {
    guiTests.append(t);
}

void GUIMultiTest::newState(State newState, State prevState) {

    Q_ASSERT(state == prevState);
    state = newState;
}

void GUIMultiTest::setWaiting(int ms) {

    newState(WAITING, RUNNING);
    waiting = ms;
}

void GUIMultiTest::sl_subTestFinished(GUITest *t) {

    if (GUIMultiTest* multiTest = qobject_cast<GUIMultiTest*>(t)) {
        if (!os.hasError()) {
            os.setError(multiTest->os.getError());
        }
    }

    setWaiting(1000); // wait after every subtest
    t->deleteLater();
}

void GUIMultiTest::allTestsFinished() {

    newState(FINISHED, IDLE);
    timer.stop();

    execute(os);
    testFinished();
}

GUITest* GUIMultiTest::getNextTest() {

    return guiTests.isEmpty() ? NULL : guiTests.takeFirst();
}

void GUIMultiTest::runNextTest() {

    GUITest* t = getNextTest();
    if (t) {
        newState(RUNNING, IDLE);
        connect(t, SIGNAL(finished(GUITest*)), this, SLOT(sl_subTestFinished(GUITest*)), Qt::QueuedConnection);
        t->run(os);
    }
    else {
        allTestsFinished();
    }
}

void GUIMultiTest::updateWaiting() {

    waiting -= timerInterval;
    if (waiting <= 0) {
        newState(IDLE, WAITING);
    }
}

void GUIMultiTest::sl_update() {

    switch (state) {
        case IDLE:
            runNextTest();
            break;

        case WAITING:
            updateWaiting();
            break;

        default:
        case RUNNING:
            break;
    }
}

} // namespace
