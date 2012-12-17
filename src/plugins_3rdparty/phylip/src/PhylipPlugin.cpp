/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "PhylipPlugin.h"

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/GObjectTypes.h>

#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/DialogUtils.h>

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

const QString PhylipPlugin::PHYLIP_NEIGHBOUR_JOIN("PHYLIP Neighbor Joining");

PhylipPlugin::PhylipPlugin() 
: Plugin(tr("PHYLIP"), tr("PHYLIP (the PHYLogeny Inference Package) is a package of programs for inferring phylogenies (evolutionary trees)."
         " Original version at: http://evolution.genetics.washington.edu/phylip.html"), false)
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
