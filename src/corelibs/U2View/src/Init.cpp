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

#include "Init.h"

#include <U2Core/AppContext.h>

#include <U2Gui/OPWidgetFactory.h>
#include <U2Gui/OPWidgetFactoryRegistry.h>

#include <U2View/AnnotHighlightWidgetFactory.h>
#include <U2View/AssemblyInfoWidget.h>
#include <U2View/AssemblyNavigationWidget.h>
#include <U2View/AssemblySettingsWidget.h>
#include <U2View/FindPatternWidgetFactory.h>
#include <U2View/PairAlignFactory.h>
#include <U2View/RefSeqCommonWidget.h>
#include <U2View/SequenceInfoFactory.h>
#include <U2View/SeqStatisticsWidgetFactory.h>
#include <U2View/MSAGeneralTabFactory.h>
#include <U2View/MSAHighlightingTabFactory.h>
#include "ov_msa/TreeOptions/TreeOptionsWidgetFactory.h"
#include <U2View/DasWidgetFactory.h>


namespace U2 {

void Init::initOptionsPanels()
{
    OPWidgetFactoryRegistry *opWidgetFactoryRegistry = AppContext::getOPWidgetFactoryRegistry();
    OPCommonWidgetFactoryRegistry *opCommonWidgetFactoryRegistry = AppContext::getOPCommonWidgetFactoryRegistry();

    // Sequence View groups
    opWidgetFactoryRegistry->registerFactory(new FindPatternWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AnnotHighlightWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new SequenceInfoFactory());

    opWidgetFactoryRegistry->registerFactory(new DasWidgetFactory());


    // Assembly Browser groups
    opWidgetFactoryRegistry->registerFactory(new AssemblyNavigationWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AssemblyInfoWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AssemblySettingsWidgetFactory());


    //MSA groups
    MSAGeneralTabFactory *msaGeneralTabFactory = new MSAGeneralTabFactory();
    QString msaGeneralId = msaGeneralTabFactory->getOPGroupParameters().getGroupId();
    opWidgetFactoryRegistry->registerFactory(msaGeneralTabFactory);

    MSAHighlightingFactory *msaHighlightingFactory = new MSAHighlightingFactory();
    QString msaHighlightingId = msaHighlightingFactory->getOPGroupParameters().getGroupId();
    opWidgetFactoryRegistry->registerFactory(msaHighlightingFactory);

    opWidgetFactoryRegistry->registerFactory(new PairAlignFactory());
    opWidgetFactoryRegistry->registerFactory(new MSATreeOptionsWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AddTreeWidgetFactory());

    SeqStatisticsWidgetFactory *msaSeqStatisticvsFactory = new SeqStatisticsWidgetFactory();
    QString msaSeqStatisticsId = msaSeqStatisticvsFactory->getOPGroupParameters().getGroupId();
    opWidgetFactoryRegistry->registerFactory(msaSeqStatisticvsFactory);

    // MSA common widgets
    QList<QString> groupIds;
    groupIds << msaHighlightingId << msaSeqStatisticsId << msaGeneralId;
    RefSeqCommonWidgetFactory *refSeqCommonWidget = new RefSeqCommonWidgetFactory(groupIds);
    opCommonWidgetFactoryRegistry->registerFactory(refSeqCommonWidget);


    //Tree View groups
    opWidgetFactoryRegistry->registerFactory(new TreeOptionsWidgetFactory());

}

} // namespace
