#include "GUITestBase.h"


namespace U2 {

bool GUITestBase::registerTest(GUITest *t) {
    if(!findTestByName(t->getName())) {
        tests.insert(t->getName(), t);
        return true;
    }
    return false;
}
GUITest *GUITestBase::findTestByName(const QString &name) {
    QMap<QString, GUITest*>::const_iterator i = tests.find(name);
    if(i == tests.end()) {
        return NULL;
    } else {
        return i.value();
    }
}
QList<GUITest*> GUITestBase::getTests() {
    return tests.values();
}




}
