#ifndef APITESTSPLUGIN_H
#define APITESTSPLUGIN_H

#include <U2Core/PluginModel.h>
#include <U2Test/APITestRunner.h>

namespace U2 {

class ApiTestsPlugin : public Plugin {
    Q_OBJECT
public:
    ApiTestsPlugin();
};

class APITestRunnerImpl : public APITestRunner {
public:
    virtual void launchTests(const QStringList& suiteUrls);
private:
    static void parseTestNames(const QStringList& urls, QStringList& result);
};

} // namespace U2

#endif // APITESTSPLUGIN_H
