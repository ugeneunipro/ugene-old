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
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2CrossDatabaseReferenceDbi.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2AttributeUtils.h>
#include <U2Core/U2SafePoints.h>

#include <memory>

namespace U2 {

//==============================================================================
// AssemblyModel
//==============================================================================

AssemblyModel::AssemblyModel(const DbiHandle & dbiHandle_) : 
cachedModelLength(NO_VAL), cachedModelHeight(NO_VAL), referenceDbi(0), dbiHandle(dbiHandle_), assemblyDbi(0), 
refSeqDbiHandle(0), loadingReference(false), refDoc(0), md5Retrieved(false) {
    Project * prj = AppContext::getProject();
    if(prj != NULL) {
        connect(prj, SIGNAL(si_documentRemoved(Document*)), SLOT(sl_referenceDocRemoved(Document*)));
        connect(prj, SIGNAL(si_documentAdded(Document*)), SLOT(sl_referenceDocAdded(Document*)));
    }
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
        // try to set length from attributes
        U2AttributeDbi * attributeDbi = dbiHandle.dbi->getAttributeDbi();
        U2OpStatusImpl status;
        static const QByteArray REFERENCE_ATTRIBUTE_NAME("reference_length_attribute");
        if(attributeDbi != NULL) {
            cachedModelLength = U2AttributeUtils::findIntegerAttribute(attributeDbi, assembly.id, REFERENCE_ATTRIBUTE_NAME, NO_VAL, os);
        }
        // if cannot from attributes -> set from reference or max end pos
        if(cachedModelLength == NO_VAL) {
            LOG_OP(status);
            qint64 refLen = hasReference() ? reference.length : 0;
            qint64 assLen = assemblyDbi->getMaxEndPos(assembly.id, status);
            cachedModelLength = qMax(refLen, assLen);
        }
    }
    return cachedModelLength;
}

QByteArray AssemblyModel::getReferenceMd5(U2OpStatus& os) {
    if(!md5Retrieved) {
        md5Retrieved = true;
        U2AttributeDbi * attributeDbi = dbiHandle.dbi->getAttributeDbi();
        static const QByteArray MD5_ATTRIBUTE_NAME("reference_md5_attribute");
        if (attributeDbi != NULL) {
            referenceMd5 = U2AttributeUtils::findByteArrayAttribute(attributeDbi, assembly.id, MD5_ATTRIBUTE_NAME, QByteArray(), os);
        }
    }
    return referenceMd5;
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
        SAFE_POINT_OP(status,); 
        
        // 2. find project and load reference doc to project
        Project * prj = AppContext::getProject();
        SAFE_POINT(prj!=NULL, tr("No active project found!"), );

        refDoc = prj->findDocumentByURL(crossRef.dataRef.dbiId);
        Task * t = NULL;
        if( refDoc != NULL ) { // document already in project, load if it is not loaded
            if(refDoc->isLoaded()) {
                sl_referenceLoaded();
            } else {
                t = new LoadUnloadedDocumentTask(refDoc);
            }
        } else { // no document at project -> create doc, add it to project and load it
            t = createLoadReferenceAndAddtoProjectTask(crossRef);
            SAFE_POINT(t, "Failed to load reference sequence!",);
        }
        
        // 3. watch load-unload doc
        connect(refDoc, SIGNAL(si_loadedStateChanged()), SLOT(sl_referenceDocLoadedStateChanged()));
        
        // 4. run task and wait for finished in referenceLoaded()
        if(t != NULL) {
            startLoadReferenceTask(t);
        }
    }
}

Task * AssemblyModel::createLoadReferenceAndAddtoProjectTask(const U2CrossDatabaseReference& ref) {
    // hack: factoryId in FileDbi looks like FileDbi_formatId
    DocumentFormatId fid = ref.dataRef.factoryId.mid(ref.dataRef.factoryId.indexOf("_") + 1);
    DocumentFormat * df = AppContext::getDocumentFormatRegistry()->getFormatById(fid);
    SAFE_POINT(df, QString("Document format is not supported? %1").arg(fid), NULL);
    
    QString url = ref.dataRef.dbiId;
    IOAdapterId iofId = BaseIOAdapters::url2io(url);
    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(iofId);
    SAFE_POINT(iof, QString("IO-factory is unknown? %1, url: %2").arg(iofId).arg(url), NULL);

    refDoc = new Document(df, iof, url);
    Task * t = new LoadUnloadedDocumentTask(refDoc);
    t->addSubTask(new AddDocumentTask(refDoc));
    t->setMaxParallelSubtasks(1);
    return t;
}

void AssemblyModel::startLoadReferenceTask(Task * t) {
    assert(t != NULL);
    connect(new TaskSignalMapper(t), SIGNAL(si_taskSucceeded(Task*)), SLOT(sl_referenceLoaded()));
    loadingReference = true;
    emit si_referenceChanged();
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

// when reference doc removed from project
void AssemblyModel::sl_referenceDocRemoved(Document* d) {
    if(d != NULL && d == refDoc) {
        refDoc->disconnect(SIGNAL(si_loadedStateChanged()));
        cleanup();
        emit si_referenceChanged();
    }
}

// when reference doc added to project
void AssemblyModel::sl_referenceDocAdded(Document * d) {
    SAFE_POINT(d, "Reference document is NULL!", );

    if(refDoc.isNull() && !assembly.referenceId.isEmpty()) {
        U2OpStatusImpl status;
        U2CrossDatabaseReference ref = dbiHandle.dbi->getCrossDatabaseReferenceDbi()->getCrossReference(assembly.referenceId, status);
        SAFE_POINT_OP(status,);
        
        if(ref.dataRef.dbiId == d->getURLString()) {
            if(!d->isLoaded()) {
                startLoadReferenceTask(new LoadUnloadedDocumentTask(refDoc = d));
            } else {
                assert(false);
            }
        }
    }
}

// when load-unload document
void AssemblyModel::sl_referenceDocLoadedStateChanged() {
    Document * doc = qobject_cast<Document*>(sender());
    SAFE_POINT(doc, "Reference document is NULL!", );
    
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
    if(status.hasError()) {
        LOG_OP(status);
        cleanup();
        return;
    }
    U2SequenceDbi * seqDbi = refSeqDbiHandle->dbi->getSequenceDbi();
    if(seqDbi != NULL) {
        U2Sequence refSeq = seqDbi->getSequenceObject(ref.dataRef.entityId, status);
        SAFE_POINT_OP(status,);
        
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
    if(refDoc.isNull()) {
        Project * p = AppContext::getProject();
        if(p != NULL) {
            refDoc = p->findDocumentByURL(seq.dbiId);
        }
    }
    reference = seq;
    referenceDbi = dbi;
    emit si_referenceChanged();
}

QByteArray AssemblyModel::getReferenceRegion(const U2Region& region, U2OpStatus& os) {
    SAFE_POINT(!refDoc.isNull() && refDoc->isLoaded(), "Reference document is not ready!", QByteArray());

    return referenceDbi->getSequenceData(reference.id, region, os);
}

void AssemblyModel::associateWithReference(const U2CrossDatabaseReference & ref) {
    assert(hasReference());
    assert(assemblyDbi != NULL);
    // save cross reference id to assembly
    assembly.referenceId = ref.id;
    U2OpStatusImpl status;
    assemblyDbi->updateAssemblyObject(assembly, status);
    LOG_OP(status);
}

} // U2
