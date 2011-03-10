#ifndef _U2_TEST_SCRIPT_WRAPPER_H_
#define _U2_TEST_SCRIPT_WRAPPER_H_

#include <QtScript>
#include <QTest>
#include <QObject>

namespace U2 {
    
class GTestScriptWrapper {
public:
    GTestScriptWrapper();

    static QScriptValue qtestMouseClickE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestMouseDClickE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestMouseMoveE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestMousePressE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestMouseReleaseE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestKeyClickE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestKeyClicksE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestKeyEventE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestKeyPressE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestKeyReleaseE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestSleepE(QScriptContext *ctx, QScriptEngine *eng);
    static QScriptValue qtestWaitE(QScriptContext *ctx, QScriptEngine *eng);

    static void setQTest(QScriptEngine* curEng);
};
}//namespace

#endif