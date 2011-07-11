/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "BowtieConstants.h"
#include "BowtiePlugin.h"
#include "BowtieTask.h"
#include "BowtieSettingsWidget.h"

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Algorithm/DnaAssemblyAlgRegistry.h>

#include <U2Lang/WorkflowEnv.h>

#include <U2Core/GAutoDeleteList.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

#include "bowtie_tests/bowtieTests.h"
#include "BowtieWorker.h"

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    BowtiePlugin * plug = new BowtiePlugin();
    return plug;
}

const QString BowtiePlugin::EBWT_INDEX_TYPE_ID("ebwt");

DataTypePtr BowtiePlugin::EBWT_INDEX_TYPE()
{
	DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
	assert(dtr);
	static bool startup = true;
	if (startup)
	{
		dtr->registerEntry(DataTypePtr(new DataType(EBWT_INDEX_TYPE_ID, tr("EBWT index"), tr("EBWT Index for bowtie"))));
		startup = false;
	}
	return dtr->getById(EBWT_INDEX_TYPE_ID);
}

class BWTGuiExtFactory : public DnaAssemblyGUIExtensionsFactory {
public:
    BWTGuiExtFactory(){};
    DnaAssemblyAlgorithmMainWidget* createMainWidget(QWidget* parent) {return new BowtieSettingsWidget(parent);}
    DnaAssemblyAlgorithmBuildIndexWidget* createBuildIndexWidget(QWidget* ) { assert(0); return NULL;}
    bool hasMainWidget() {return true;}
    bool hasBuildIndexWidget() {return false;}
};
   
BowtiePlugin::BowtiePlugin() : Plugin(tr("Bowtie"), 
		 tr("An ultrafast memory-efficient short read aligner, http://bowtie-bio.sourceforge.net")),
         ctx(NULL)
{
	GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = BowtieTests ::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }

	DnaAssemblyAlgRegistry* registry = AppContext::getDnaAssemblyAlgRegistry();
    
    bool guiMode = AppContext::getMainWindow()!=NULL;
    DnaAssemblyGUIExtensionsFactory* guiFactory = guiMode ? new BWTGuiExtFactory(): NULL;
    DnaAssemblyAlgorithmEnv* algo = new DnaAssemblyAlgorithmEnv(BowtieBaseTask::taskName, new BowtieBaseTask::Factory(), guiFactory, true, false);
    bool res = registry->registerAlgorithm(algo);
    Q_UNUSED(res);
    assert(res);

	LocalWorkflow::BowtieWorkerFactory::init();
	LocalWorkflow::BowtieBuildWorkerFactory::init();
	LocalWorkflow::BowtieIndexReaderWorkerFactory::init();
}

BowtiePlugin::~BowtiePlugin() {
    DnaAssemblyAlgRegistry* registry = AppContext::getDnaAssemblyAlgRegistry();
    DnaAssemblyAlgorithmEnv* algo = registry->unregisterAlgorithm(BowtieBaseTask::taskName);
    assert(algo!=NULL);
    delete algo;
}

}//namespace
