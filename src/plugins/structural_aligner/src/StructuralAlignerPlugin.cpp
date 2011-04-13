#include "StructuralAlignerPlugin.h"
#include "StructuralAlignerPtools.h"
#include "StructuralAlignerTests.h"

#include <U2Core/AppContext.h>
#include <U2Algorithm/StructuralAlignmentAlgorithmFactory.h>
#include <U2Algorithm/StructuralAlignmentAlgorithmRegistry.h>
#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>
#include <U2Core/GAutoDeleteList.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    StructuralAlignerPlugin *plug = new StructuralAlignerPlugin();
    return plug;
}

/* class StructuralAlignerPlugin : public Plugin */

StructuralAlignerPlugin::StructuralAlignerPlugin() : Plugin(tr("Structural Aligner"), tr("This plugin contains structural alignment algorithms (ptools)"))
{
    StructuralAlignmentAlgorithmFactory *ptools = new StructuralAlignerPtoolsFactory();
    AppContext::getStructuralAlignmentAlgorithmRegistry()->registerAlgorithmFactory(ptools, "ptools");

    // StructuralAligner tests
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = StructualAlignerTests::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) {
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
}

StructuralAlignerPlugin::~StructuralAlignerPlugin()
{
}

/* class StructualAlignerTests */

QList<XMLTestFactory*> StructualAlignerTests ::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(Gtest_StructuralAlignerTask::createFactory());
    return res;
}

}   // namespace U2

