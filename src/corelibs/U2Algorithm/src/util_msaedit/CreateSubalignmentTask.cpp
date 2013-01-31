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

#include "CreateSubalignmentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/Log.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/U2SafePoints.h>

namespace U2{

CreateSubalignmentTask::CreateSubalignmentTask(MAlignmentObject* maObj, const CreateSubalignmentSettings& settings )
:DocumentProviderTask(tr("Create sub-alignment: %1").arg(maObj->getDocument()->getName()), TaskFlags_NR_FOSCOE), 
origMAObj(maObj), cfg(settings)
{
    origDoc = maObj->getDocument();
    createCopy = cfg.url != origDoc->getURL() || cfg.url.isEmpty();
}

void CreateSubalignmentTask::prepare() {
    QString ext = cfg.url.completeFileSuffix();

    DocumentFormatRegistry *dfr = AppContext::getDocumentFormatRegistry();
    DocumentFormat *dfd = dfr->getFormatById(BaseDocumentFormats::CLUSTAL_ALN);

    foreach(const DocumentFormatId &dfId, dfr->getRegisteredFormats()) {
        DocumentFormat *df = dfr->getFormatById(dfId);
        foreach(const QString &dfExt, df->getSupportedDocumentFileExtensions()) {
            if (ext.endsWith(dfExt)) {
                dfd = df;
                break;
            }
        }
    }

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(cfg.url));
    if (createCopy) {
        resultDocument = dfd->createNewLoadedDocument(iof, cfg.url, stateInfo, origDoc->getGHintsMap());
        CHECK_OP(stateInfo, );

        MAlignment msa = origMAObj->getMAlignment();
        U2EntityRef msaRef = MAlignmentImporter::createAlignment(resultDocument->getDbiRef(), origMAObj->getMAlignment(), stateInfo);
        CHECK_OP(stateInfo, );

        resultMAObj = new MAlignmentObject(msa.getName(), msaRef, origMAObj->getGHintsMap());

        resultDocument->addObject(resultMAObj);
        GObjectUtils::updateRelationsURL(resultMAObj, origDoc->getURL(), cfg.url);
    } else {
        CHECK_EXT(origDoc->isStateLocked(), setError(tr("Document is locked: %1").arg(origDoc->getURLString())), );
        resultDocument = origDoc;
        resultMAObj = origMAObj;
        docOwner = false;
    }

    //TODO: add "remove empty rows and columns" flag to crop function
    QSet<QString> rowNames;
    foreach (const QString& name, cfg.seqNames) {
        rowNames.insert(name);
    }
    resultMAObj->crop(cfg.window, rowNames);
    
    if (cfg.saveImmediately) {
        addSubTask(new SaveDocumentTask(resultDocument, iof));
    }
 }

}
