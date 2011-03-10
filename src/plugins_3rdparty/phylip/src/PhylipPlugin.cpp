#include "PhylipPlugin.h"

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/GObjectTypes.h>

#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/GUIUtils.h>
#include <U2Misc/DialogUtils.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

#include <QtGui/QDialog>
#include <QtGui/QFileDialog>

#include "PhylipPluginTests.h"
#include "NeighborJoinAdapter.h"
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
	PhylipPlugin * plug = new PhylipPlugin();
	return plug;
}

const QString PhylipPlugin::PHYLIP_NEIGHBOUR_JOIN("Phylip Neighbor Join");

PhylipPlugin::PhylipPlugin() 
: Plugin(tr("Phylip plugin"), tr("PHYLIP(the PHYLogeny Inference Package) is a package of programs \
                                 for inferring phylogenies (evolutionary trees).\nOriginal version\
                                 at:http://evolution.genetics.washington.edu/phylip.html"))
{

    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    registry->registerPhyTreeGenerator(new NeighborJoinAdapter(), PHYLIP_NEIGHBOUR_JOIN);

    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = PhylipPluginTests::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
}

PhylipPlugin::~PhylipPlugin() {
    //nothing to do
}


}//namespace
