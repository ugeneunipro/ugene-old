#ifndef _U2_TEST_FRAMWORK_COMPONENTS_
#define _U2_TEST_FRAMWORK_COMPONENTS_

#include "GTest.h"

namespace U2 {

class U2TEST_EXPORT GTestFormatRegistry : public QObject {
    Q_OBJECT
public:
    GTestFormatRegistry();
    ~GTestFormatRegistry();
    
    bool registerTestFormat(GTestFormat* f);

    bool unregisterTestFormat(GTestFormat* f);

    QList<GTestFormat*> getFormats() const {return formats;}

    GTestFormat* findFormat(const GTestFormatId& id);

private:
    QList<GTestFormat*> formats;

};

class U2TEST_EXPORT TestFramework {
public:
    static const int TEST_TIMEOUT_DEFAULT = 0;
    static const QString TEST_TIMEOUT_CMD_OPTION;
    
private:
    static bool helpRegistered;
    static void setTRHelpSections();
    
public:
    TestFramework();
    GTestFormatRegistry* getTestFormatRegistry() {return &formatRegistry;}

private:
    void setTestRunnerSettings();
    
private:
    GTestFormatRegistry formatRegistry;
};

}//namespace

#endif
