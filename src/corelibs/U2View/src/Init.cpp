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

#include "Init.h"

#include <U2Core/AppContext.h>

#include <U2Gui/OPWidgetFactoryRegistry.h>

#include <U2View/AnnotHighlightWidgetFactory.h>
#include <U2View/AssemblyInfoWidget.h>
#include <U2View/AssemblyNavigationWidget.h>
#include <U2View/AssemblySettingsWidget.h>
#include <U2View/FindPatternWidgetFactory.h>
#include <U2View/SequenceInfoFactory.h>


namespace U2 {

void Init::initOptionsPanels()
{
    OPWidgetFactoryRegistry* opWidgetFactoryRegistry = AppContext::getOPWidgetFactoryRegistry();

    // Sequence View groups
    opWidgetFactoryRegistry->registerFactory(new SequenceInfoFactory());
    opWidgetFactoryRegistry->registerFactory(new FindPatternWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AnnotHighlightWidgetFactory());

    // Assembly Browser groups
    opWidgetFactoryRegistry->registerFactory(new AssemblyNavigationWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AssemblySettingsWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AssemblyInfoWidgetFactory());
}

} // namespace
