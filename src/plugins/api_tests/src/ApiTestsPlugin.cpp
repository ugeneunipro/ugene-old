#include "ApiTestsPlugin.h"
#include "ApiTest.h"

#include <U2Core/AppContext.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTestFrameworkComponents.h>


namespace U2 {

extern "C" Q_DECL_EXPORT U2::Plugin *U2_PLUGIN_INIT_FUNC()
{
    return new ApiTestsPlugin();
}

ApiTestsPlugin::ApiTestsPlugin() :
Plugin("UGENE 2.0 API tests", "Tests for UGENE 2.0 public API") {
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);
    XMLTestFactory* f = GTest_APITest::createFactory();
    bool res = xmlTestFormat->registerTestFactory(f);
    assert(res); Q_UNUSED(res);
}

} // namespace U2
