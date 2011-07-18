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

#include "CreateSubalignmentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Log.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/BaseDocumentFormats.h>

namespace U2{

CreateSubalignmentTask::CreateSubalignmentTask(MAlignmentObject* _maObj, const CreateSubalignmentSettings& settings )
: Task(tr("Extract selected as MSA task"), TaskFlags_NR_FOSCOE),
maObj(_maObj), cfg(settings)
{
    curDoc = maObj->getDocument();
    newDoc = NULL;
    if (cfg.url == curDoc->getURL() || cfg.url.isEmpty()) {
        saveToAnother = false;
    }
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

    newDoc = curDoc;

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(cfg.url));
    if (saveToAnother) {
        QList<GObject*> GObjList = curDoc->getObjects();
        newDoc = dfd->createNewDocument(iof, cfg.url, curDoc->getGHintsMap());
        foreach(GObject* go, GObjList){
            GObject *cl = go->clone();
            newDoc->addObject(cl);
            if (go == maObj){
                maObj = static_cast<MAlignmentObject*> (cl);
            }
        }
        foreach(GObject* o, newDoc->getObjects()) {
            GObjectUtils::updateRelationsURL(o, curDoc->getURL(), cfg.url);
        }
    } else {
        if(newDoc->isStateLocked()) {
            coreLog.error(tr("Document is locked"));
            return;
        }
    }

    //TODO: add "remove empty rows and columns" flag to crop function
    QSet<QString> rowNames;
    foreach (const QString& name, cfg.seqNames) {
        rowNames.insert(name);
    }
    maObj->crop(cfg.window, rowNames);
    
    if (cfg.saveImmediately) {
        addSubTask(new SaveDocumentTask(newDoc, iof, cfg.url));
    }
    

 }

void CreateSubalignmentTask::cleanup() {
    if (newDoc != NULL) {
        delete newDoc;
        newDoc = NULL;
    }
}

Document* CreateSubalignmentTask::takeDocument() {
    Document* doc = newDoc;
    newDoc = NULL;
    return doc;
}

CreateSubalignmentTask::~CreateSubalignmentTask() {
    cleanup();
}



}
