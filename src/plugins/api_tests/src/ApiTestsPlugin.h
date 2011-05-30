#ifndef APITESTSPLUGIN_H
#define APITESTSPLUGIN_H

#include <U2Core/PluginModel.h>
#include <U2Test/XMLTestUtils.h>


namespace U2 {

class ApiTestsPlugin : public Plugin {
    Q_OBJECT
public:
    ApiTestsPlugin();
};

class GTest_APITest : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_APITest, "gtest", TaskFlags_FOSCOE);

    void prepare();
    void run();
    void cleanup();

private:
    QString tcase;
    QStringList excluded;
    QStringList included;
};

} // namespace U2

#endif // APITESTSPLUGIN_H
