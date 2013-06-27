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

#include "snp_toolbox/SNPToolboxWorker.h"
#include "SNPReportWriter.h"
#include "rsnp_tools/RSnpToolsWorker.h"
#include "tata_box_analysis/AnalyzeTataBoxesWorker.h"

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    SNPEffectPlugin* plug = new SNPEffectPlugin();
    return plug;
}

    
SNPEffectPlugin::SNPEffectPlugin() : Plugin(tr("SNP Effect Plugin"), tr("Evaluates effect of human genome variations and finds relations with diseases")){
    LocalWorkflow::SNPToolboxWorkerFactory::init();
    LocalWorkflow::SNPReportWriterFactory::init();
    LocalWorkflow::RSnpToolsWorkerFactory::init();
    LocalWorkflow::AnalyzeTataBoxesWorkerFactory::init( );
}

}//namespace
