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

#include "FileDbiPlugin.h"
#include "FileDbi.h"

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2DbiRegistry.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    FileDbiPlugin* plug = new FileDbiPlugin();
    return plug;
}

FileDbiPlugin::FileDbiPlugin() : Plugin(tr("Plain files support in database"), tr("Adds support for using plain files in database")) {
    DocumentFormatConstraints constr;
    constr.supportedObjectTypes.insert(GObjectTypes::SEQUENCE);
    foreach(const DocumentFormatId & fid, AppContext::getDocumentFormatRegistry()->selectFormats(constr)) {
        AppContext::getDbiRegistry()->registerDbiFactory(new FileDbiFactory(fid));
    }
}

} // U2
