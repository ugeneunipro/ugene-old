/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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




#include "SNPEffectPlugin.h"

#include "SNPEffectTests.h"

#include "BaseRequestForSnpWorker.h"

#include "snp_toolbox/SNPToolboxWorker.h"
#include "SNPReportWriter.h"
#include "prot_stability_1d/ProtStability1DWorker.h"
#include "prot_stability_3d/ProtStability3DWorker.h"
#include "rsnp_tools/RSnpToolsWorker.h"
#include "snp_chip_tools/SnpChIpToolsWorker.h"
#include "snp2pdb_site/Snp2PdbSiteWorker.h"
#include "tata_box_analysis/AnalyzeTataBoxesWorker.h"

#include <U2Core/GAutoDeleteList.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    SNPEffectPlugin* plug = new SNPEffectPlugin();
    return plug;
}

    
SNPEffectPlugin::SNPEffectPlugin() : Plugin(tr("SNP Effect Plugin"), tr("Evaluates effect of human genome variations and finds relations with diseases")){

    U2DataPathRegistry* dpr = AppContext::getDataPathRegistry();
    if (dpr){
        U2DataPath* dp = new U2DataPath(LocalWorkflow::BaseRequestForSnpWorker::DB_SEQUENCE_PATH, QString(PATH_PREFIX_DATA)+QString(":")+LocalWorkflow::BaseRequestForSnpWorker::DB_FILE, false);
        dpr->registerEntry(dp);
    }

    LocalWorkflow::SNPToolboxWorkerFactory::init();
    LocalWorkflow::SNPReportWriterFactory::init();
    LocalWorkflow::ProtStability1DWorkerFactory::init();
    LocalWorkflow::ProtStability3DWorkerFactory::init();
    LocalWorkflow::Snp2PdbSiteWorkerFactory::init();
    LocalWorkflow::SnpChipToolsWorkerFactory::init();
    LocalWorkflow::RSnpToolsWorkerFactory::init();
    LocalWorkflow::AnalyzeTataBoxesWorkerFactory::init( );

    
    //tests
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = SNPResultTests::createTestFactories();


    foreach(XMLTestFactory* f, l->qlist) {
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
}

}//namespace
