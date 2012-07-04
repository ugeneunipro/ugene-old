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

#include "DNAExportPlugin.h"
#include "DNAExportPluginTests.h"

#include "ExportProjectViewItems.h"
#include "ExportSequenceViewItems.h"
#include "ExportAlignmentViewItems.h"
#include "ImportQualityScoresWorker.h"
#include "ExportQualityScoresWorker.h"
#include "WriteAnnotationsWorker.h"
#include "DNASequenceGeneratorDialog.h"
#include "GenerateDNAWorker.h"

#include <U2Core/AppContext.h>
#include <U2Core/GAutoDeleteList.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTestFrameworkComponents.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    DNAExportPlugin* plug = new DNAExportPlugin();
    return plug;
}

DNAExportPlugin::DNAExportPlugin() : Plugin(tr("DNA export"), tr("Export and import support for DNA & protein sequences")) {
    if (AppContext::getMainWindow()) {
        services.push_back(new DNAExportService());
        QAction* a = new QAction(tr("Generate Sequence..."), this);
        a->setObjectName("Generate Sequence");
        connect(a, SIGNAL(triggered()), SLOT(sl_generateSequence()));
        QMenu* toolsMenu = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS);
        toolsMenu->addAction(a);
    }

    //tests
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = DNAExportPluginTests::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        assert(res); Q_UNUSED(res);
    }
    
    LocalWorkflow::ImportPhredQualityWorkerFactory::init();
    LocalWorkflow::ExportPhredQualityWorkerFactory::init();
    LocalWorkflow::WriteAnnotationsWorkerFactory::init();
    LocalWorkflow::GenerateDNAWorkerFactory::init();

}

void DNAExportPlugin::sl_generateSequence() {
    DNASequenceGeneratorDialog dlg;
    dlg.exec();
}

//////////////////////////////////////////////////////////////////////////
// Service
DNAExportService::DNAExportService() 
: Service(Service_DNAExport, tr("DNA export service"), tr("Export and import support for DNA & protein sequences"), 
          QList<ServiceType>() << Service_ProjectView)
{
    projectViewController = NULL;
    sequenceViewController = NULL;
    alignmentViewController = NULL;
}

void DNAExportService::serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged) {
    Q_UNUSED(oldState);

    if (!enabledStateChanged) {
        return;
    }
    if (isEnabled()) {
        projectViewController = new ExportProjectViewItemsContoller(this);
        sequenceViewController = new ExportSequenceViewItemsController(this);
        sequenceViewController->init();
        alignmentViewController = new ExportAlignmentViewItemsController(this);
        alignmentViewController->init();
    } else {
        delete projectViewController; projectViewController = NULL;
        delete sequenceViewController; sequenceViewController = NULL;
        delete alignmentViewController; alignmentViewController = NULL;
    }
}


}//namespace
