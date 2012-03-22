#include "ApiTestsPlugin.h"
#include "UnitTestSuite.h"

#include <U2Core/AppContext.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTestFrameworkComponents.h>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2SafePoints.h>

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

	XMLTestFactory* f = UnitTestSuite::createFactory();
    bool res = xmlTestFormat->registerTestFactory(f);
    SAFE_POINT(res, "API tests is not registered",);
	Q_UNUSED(res);
}

} // namespace U2
