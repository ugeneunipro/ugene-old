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

#include "AssemblyModel.h"
#include "AssemblyBrowser.h"

#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2CrossDatabaseReferenceDbi.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/TaskSignalMapper.h>

#include <memory>

namespace U2 {

//==============================================================================
// AssemblyModel
//==============================================================================

AssemblyModel::AssemblyModel(const DbiHandle & dbiHandle_) : 
cachedModelLength(NO_VAL), cachedModelHeight(NO_VAL), referenceDbi(0), dbiHandle(dbiHandle_), assemblyDbi(0), 
refSeqDbiHandle(0), loadingReference(false), refDoc(0) {
}

AssemblyModel::~AssemblyModel() {
    cleanup();
}

void AssemblyModel::cleanup() {
    if(refSeqDbiHandle != NULL) {
        delete refSeqDbiHandle;
        refSeqDbiHandle = NULL;
        referenceDbi = NULL;
        reference.length = 0;
        refDoc = NULL;
    }
}

bool AssemblyModel::isEmpty() const {
    return assemblyDbi == NULL;
}

QList<U2AssemblyRead> AssemblyModel::getReadsFromAssembly(const U2Region & r, qint64 minRow, qint64 maxRow, U2OpStatus & os) {
    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > it(assemblyDbi->getReadsByRow(assembly.id, r, minRow, maxRow, os));
    return U2DbiUtils::toList(it.get());
}

qint64 AssemblyModel::countReadsInAssembly(const U2Region & r, U2OpStatus & os) {
    return assemblyDbi->countReads(assembly.id, r, os);
}

qint64 AssemblyModel::getModelLength(U2OpStatus & os) {
    if(NO_VAL == cachedModelLength) {
        //TODO: length must be calculated as length of the reference sequence or
        //if there is no reference, as maximum of all assembly lengths in the model
        qint64 refLen = hasReference() ? reference.length : 0;
        qint64 assLen = assemblyDbi->getMaxEndPos(assembly.id, os);
        cachedModelLength = qMax(refLen, assLen);
        //dbiHandle.dbi->getAttributeDbi()->getInt32Attribute("reference_length");
    }
    return cachedModelLength;
}

qint64 AssemblyModel::getModelHeight(U2OpStatus & os) {
    if(NO_VAL == cachedModelHeight) {
        // TODO: get rid of this? Use predefined max value?
        qint64 zeroAsmLen = assemblyDbi->getMaxEndPos(assembly.id, os); 
        //TODO: model height should be calculated as sum of all assemblies ? 
        //Or consider refactoring to getHeightOfAssembly(int assIdx, ...)
        cachedModelHeight = assemblyDbi->getMaxPackedRow(assembly.id, U2Region(0, zeroAsmLen), os);
        return cachedModelHeight;
    }
    return cachedModelHeight;
}

void AssemblyModel::setAssembly(U2AssemblyDbi * dbi, const U2Assembly & assm) {
    assert(dbi != NULL);
    assert(assemblyDbi == NULL);
    assemblyDbi = dbi;
    assembly = assm;
    
    // check if have reference
    if(!assembly.referenceId.isEmpty()) {
        // 1. get cross reference by ref id
        U2CrossDatabaseReferenceDbi * crossDbi = dbiHandle.dbi->getCrossDatabaseReferenceDbi();
        U2OpStatusImpl status;
        U2CrossDatabaseReference crossRef = crossDbi->getCrossReference(assembly.referenceId, status);
        checkAndLogError(status); 
        if(status.hasError()) {
            return;
        }

        // 2. find project and load reference doc to project
        QString url = crossRef.dataRef.dbiId;
        Project * prj = AppContext::getProject();
        if(prj == NULL) {
            assert(false);
            coreLog.error(tr("To show reference opened project needed"));
            return;
        }
        refDoc = prj->findDocumentByURL(url);
        Task * t = NULL;
        if( refDoc != NULL ) { // document already in project, load if it is not loaded
            if(refDoc->isLoaded()) {
                sl_referenceLoaded();
            } else {
                t = new LoadUnloadedDocumentTask(refDoc);
            }
        } else { // no document at project -> create doc, add it to project and load it
            // hack: factoryId in FileDbi looks like FileDbi_formatId
            DocumentFormatId fid = crossRef.dataRef.factoryId.mid(crossRef.dataRef.factoryId.indexOf("_") + 1);
            DocumentFormat * df = AppContext::getDocumentFormatRegistry()->getFormatById(fid);
            if(df == NULL) {
                coreLog.error(tr("Internal error: unknown document format '%1'").arg(fid));
                return;
            }
            IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url));
            if(iof == NULL) {
                assert(false);
                coreLog.error(tr("Internal error: cannot open file '%1' for reading").arg(url));
                return;
            }
            refDoc = new Document(df, iof, url);
            t = new LoadUnloadedDocumentTask(refDoc);
            t->addSubTask(new AddDocumentTask(refDoc));
            t->setMaxParallelSubtasks(1);
        }
        
        assert(refDoc != NULL);
        // 3. watch for document removed from project and load-unload
        connect(refDoc, SIGNAL(si_loadedStateChanged()), SLOT(sl_referenceDocLoadedStateChanged()));
        connect(prj, SIGNAL(si_documentRemoved(Document*)), SLOT(sl_referenceDocRemoved(Document*)));
        if(t != NULL) {
            // 4. run task and wait for finished in referenceLoaded()
            connect(new TaskSignalMapper(t), SIGNAL(si_taskSucceeded(Task*)), SLOT(sl_referenceLoaded()));
            loadingReference = true;
            emit si_referenceChanged();
            AppContext::getTaskScheduler()->registerTopLevelTask(t);
        }
    }
}

// when reference doc removed from project
void AssemblyModel::sl_referenceDocRemoved(Document* d) {
    if(d != NULL && d == refDoc) {
        cleanup();
        emit si_referenceChanged();
    }
}

// when load-unload document
void AssemblyModel::sl_referenceDocLoadedStateChanged() {
    Document * doc = qobject_cast<Document*>(sender());
    if(doc == NULL) {
        assert(false);
        return;
    }
    
    if(doc->isLoaded()) {
        if(!loadingReference) {
            refDoc = doc;
            sl_referenceLoaded();
        }
    } else { // refDoc unloaded
        cleanup();
        emit si_referenceChanged();
    }
}

// document is loaded and in the project -> create dbi handle and set reference
void AssemblyModel::sl_referenceLoaded() {
    U2OpStatusImpl status;
    U2CrossDatabaseReference ref = dbiHandle.dbi->getCrossDatabaseReferenceDbi()->getCrossReference(assembly.referenceId, status);
    cleanup();
    refSeqDbiHandle = new DbiHandle(ref.dataRef.factoryId, ref.dataRef.dbiId, false, status);
    checkAndLogError(status);
    if(status.hasError()) {
        cleanup();
        return;
    }
    U2SequenceDbi * seqDbi = refSeqDbiHandle->dbi->getSequenceDbi();
    if(seqDbi != NULL) {
        U2Sequence refSeq = seqDbi->getSequenceObject(ref.dataRef.entityId, status);
        checkAndLogError(status); 
        if(status.hasError()) {
            return;
        }
        setReference(seqDbi, refSeq);
    } else {
        assert(false);
    }
    loadingReference = false;
}

bool AssemblyModel::hasReference() const {
    return (bool)referenceDbi;
}

void AssemblyModel::setReference(U2SequenceDbi * dbi, const U2Sequence & seq) {
    emit si_referenceChanged();
    reference = seq;
    referenceDbi = dbi;
}

QByteArray AssemblyModel::getReferenceRegion(const U2Region& region, U2OpStatus& os) {
    if(refDoc.isNull() || !refDoc->isLoaded()) {
        assert(false);
        return QByteArray();
    }
    return referenceDbi->getSequenceData(reference.id, region, os);
}

void AssemblyModel::associateWithReference(const U2CrossDatabaseReference & ref) {
    assert(hasReference());
    assert(assemblyDbi != NULL);
    // save cross reference id to assembly
    assembly.referenceId = ref.id;
    U2OpStatusImpl status;
    assemblyDbi->updateAssemblyObject(assembly, status);
    checkAndLogError(status);
}

} // U2
