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

#include "ImportDialogsFactories.h"
#include "ImportDialogs/AceImportDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

void ImportDialogFactories::registerFactories() {
    DocumentFormatRegistry* dfRegistry = AppContext::getDocumentFormatRegistry();
    SAFE_POINT(dfRegistry, "Document format registry is NULL", );
    DocumentImportersRegistry* diRegistry = dfRegistry->getImportSupport();
    SAFE_POINT(diRegistry, "Document import registry is NULL", );

    DocumentImporter* aceImporter = diRegistry->getDocumentImporter(AceImporter::ID);
    SAFE_POINT(aceImporter, "ACE importer is NULL", );
    aceImporter->setDialogFactory(new AceImportDialogFactory());
}

ImportDialog* AceImportDialogFactory::getDialog(const QVariantMap &settings) const {
    return new AceImportDialog(settings);
}

}   // namespace U2
