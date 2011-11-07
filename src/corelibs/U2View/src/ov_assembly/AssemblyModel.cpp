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
#include <U2Core/U2AssemblyUtils.h>
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
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2AttributeUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectTypes.h>

#include <U2Gui/ObjectViewTasks.h>

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>

#include <memory>

namespace U2 {

//==============================================================================
// AssemblyModel
//==============================================================================

AssemblyModel::AssemblyModel(const DbiConnection& dbiCon_) : 
cachedModelLength(NO_VAL), cachedModelHeight(NO_VAL), assemblyDbi(0), dbiHandle(dbiCon_),
loadingReference(false), refObj(NULL), md5Retrieved(false), cachedReadsNumber(NO_VAL), speciesRetrieved(false),
uriRetrieved(false)
{
    Project * prj = AppContext::getProject();
    if (prj != NULL) {
        connect(prj, SIGNAL(si_documentRemoved(Document*)), SLOT(sl_referenceDocRemoved(Document*)));
        connect(prj, SIGNAL(si_documentAdded(Document*)), SLOT(sl_referenceDocAdded(Document*)));
    }
}

AssemblyModel::~AssemblyModel() {
    unsetReference();
}

void AssemblyModel::unsetReference() {
    refObj = NULL;
}

bool AssemblyModel::isEmpty() const {
    return assemblyDbi == NULL;
}

QList<U2AssemblyRead> AssemblyModel::getReadsFromAssembly(const U2Region & r, qint64 minRow, qint64 maxRow, U2OpStatus & os) {
    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > it(assemblyDbi->getReadsByRow(assembly.id, r, minRow, maxRow, os));
    return U2DbiUtils::toList(it.get());
}

void AssemblyModel::calculateCoverageStat(const U2Region & r, U2AssemblyCoverageStat & stat, U2OpStatus & os) {
    return assemblyDbi->calculateCoverage(assembly.id, r, stat, os);
}

const U2AssemblyCoverageStat &AssemblyModel::getCoverageStat(U2OpStatus & os) {
    QMutexLocker mutexLocker(&mutex);
    Q_UNUSED(mutexLocker);
    if(cachedCoverageStat.coverage.isEmpty()) {
        U2AttributeDbi * attributeDbi = dbiHandle.dbi->getAttributeDbi();
        if(NULL != attributeDbi) {
            static const QByteArray COVERAGE_STAT_ATTRIBUTE_NAME("coverageStat");
            U2ByteArrayAttribute attr = U2AttributeUtils::findByteArrayAttribute(attributeDbi, assembly.id, COVERAGE_STAT_ATTRIBUTE_NAME, os);
            if(!os.isCoR()) {
                if(attr.hasValidId()) {
                    // TODO: check version
                    U2AssemblyUtils::deserializeCoverageStat(attr.value, cachedCoverageStat, os);
                } else {
                    qint64 length = getModelLength(os);
                    if(!os.isCoR()) {
                        static const qint64 MAX_COVERAGE_CACHE_SIZE = 1000*1000;
                        int coverageCacheSize = (int)qMin(MAX_COVERAGE_CACHE_SIZE, length);
                        cachedCoverageStat.coverage.resize(coverageCacheSize);
                        calculateCoverageStat(U2Region(0, length), cachedCoverageStat, os);
                        if(!os.isCoR()) {
                            U2ByteArrayAttribute attribute;
                            attribute.objectId = assembly.id;
                            attribute.name = COVERAGE_STAT_ATTRIBUTE_NAME;
                            attribute.value = U2AssemblyUtils::serializeCoverageStat(cachedCoverageStat);
                            attribute.version = assembly.version;
                            U2OpStatusImpl opStatus;
                            attributeDbi->createByteArrayAttribute(attribute, opStatus);
                            LOG_OP(opStatus);
                        }
                    }
                }
            }
        } else {
            os.setError("Attribute DBI is not supported");
        }
    }
    return cachedCoverageStat;
}

U2Region AssemblyModel::getGlobalRegion() {
    U2OpStatusImpl os;
    return U2Region(0, getModelLength(os));
}

qint64 AssemblyModel::getModelLength(U2OpStatus & os) {
    if(NO_VAL == cachedModelLength) {
        // try to set length from attributes
        U2AttributeDbi * attributeDbi = dbiHandle.dbi->getAttributeDbi();
        static const QByteArray REFERENCE_ATTRIBUTE_NAME("reference_length_attribute");
        if(attributeDbi != NULL) {
            U2IntegerAttribute attr = U2AttributeUtils::findIntegerAttribute(attributeDbi, assembly.id, REFERENCE_ATTRIBUTE_NAME, os);
            LOG_OP(os);
            if(attr.hasValidId()) {
                cachedModelLength = attr.value;
            }
            // ignore incorrect attribute value and remove corrupted attribute (auto-fix incorrectly converted ugenedb)
            if(cachedModelLength == 0) {
                cachedModelLength = NO_VAL;
                coreLog.details(QString("ignored incorrect value of attribute %1: should be > 0, got %2. Bad attribute removed!").arg(QString(REFERENCE_ATTRIBUTE_NAME)).arg(cachedModelLength));
                U2AttributeUtils::removeAttribute(attributeDbi, attr.id, os);
            }
        }
        // if cannot from attributes -> set from reference or max end pos
        if(cachedModelLength == NO_VAL) {
            qint64 refLen = hasReference() ? refObj->getSequenceLength() : 0;
            qint64 assLen = assemblyDbi->getMaxEndPos(assembly.id, os);
            LOG_OP(os);
            cachedModelLength = qMax(refLen, assLen);

            // and save in attribute
            U2IntegerAttribute attr;
            U2AttributeUtils::init(attr, assembly, REFERENCE_ATTRIBUTE_NAME);
            attr.value = cachedModelLength;
            attributeDbi->createIntegerAttribute(attr, os);
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
            U2ByteArrayAttribute attr = U2AttributeUtils::findByteArrayAttribute(attributeDbi, assembly.id, MD5_ATTRIBUTE_NAME, os);
            if(attr.hasValidId()) {
                referenceMd5 = attr.value;
            }
        }
    }
    return referenceMd5;
}

qint64 AssemblyModel::getModelHeight(U2OpStatus & os) {
    if(NO_VAL == cachedModelHeight) {
        U2AttributeDbi * attributeDbi = dbiHandle.dbi->getAttributeDbi();
        static const QByteArray MAX_PROW_ATTRIBUTE_NAME("max_prow_attribute");
        if(attributeDbi != NULL) {
            U2IntegerAttribute attr = U2AttributeUtils::findIntegerAttribute(attributeDbi, assembly.id, MAX_PROW_ATTRIBUTE_NAME, os);
            LOG_OP(os);
            if(attr.hasValidId()) {
                if(attr.version == assembly.version) {
                    cachedModelHeight = attr.value;
                } else {
                    U2AttributeUtils::removeAttribute(attributeDbi, attr.id, os);
                    LOG_OP(os);
                }
            }
        }
        if(cachedModelHeight == NO_VAL) {
            // if could not get value from attribute, recompute the value...
            cachedModelHeight = assemblyDbi->getMaxPackedRow(assembly.id, U2Region(0, getModelLength(os)), os);
            LOG_OP(os);
            if(! os.isCoR()) {
                // ...and store it in a new attribure
                U2IntegerAttribute attr;
                U2AttributeUtils::init(attr, assembly, MAX_PROW_ATTRIBUTE_NAME);
                attr.value = cachedModelHeight;
                attributeDbi->createIntegerAttribute(attr, os);
            }
        }
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

        Document* refDoc = prj->findDocumentByURL(crossRef.dataRef.dbiRef.dbiId);
        Task * t = NULL;
        if( refDoc != NULL ) { // document already in project, load if it is not loaded
            if (refDoc->isLoaded()) {
                sl_referenceLoaded();
            } else {
                t = new LoadUnloadedDocumentTask(refDoc);
            }
        } else { // no document at project -> create doc, add it to project and load it
            t = createLoadReferenceAndAddToProjectTask(crossRef);
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

Task * AssemblyModel::createLoadReferenceAndAddToProjectTask(const U2CrossDatabaseReference& ref) {
    QString url = ref.dataRef.dbiRef.dbiId;

    U2OpStatus2Log os;
    Document* refDoc = ObjectViewTask::createDocumentAndAddToProject(url, AppContext::getProject(), os);
    CHECK_OP(os, NULL);

    Task * t = new LoadUnloadedDocumentTask(refDoc);
    return t;
}

void AssemblyModel::startLoadReferenceTask(Task * t) {
    assert(t != NULL);
    TaskSignalMapper * tsm = new TaskSignalMapper(t);
    connect(tsm, SIGNAL(si_taskSucceeded(Task*)), SLOT(sl_referenceLoaded()));
    connect(tsm, SIGNAL(si_taskFailed(Task*)), SLOT(sl_referenceLoadingFailed()));
    loadingReference = true;
    emit si_referenceChanged();
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void AssemblyModel::sl_referenceLoadingFailed() {
    loadingReference = false;
    emit si_referenceChanged(); // to update reference area
}


void AssemblyModel::onReferenceRemoved() {
    QMessageBox::StandardButtons fl = QMessageBox::Yes | QMessageBox::No;
    QMessageBox::StandardButton btn = QMessageBox::question(QApplication::activeWindow(), tr("Errors"),
        tr("Remove association of '%1' sequence with '%2' assembly?").arg(refObj->getGObjectName()).arg(assembly.visualName),fl, QMessageBox::Yes);
    if (btn == QMessageBox::Yes) {
        sl_unassociateReference();
    } else {
        refObj->getDocument()->disconnect(this);
        unsetReference();
        emit si_referenceChanged();
    }
}

// when reference doc removed from project
void AssemblyModel::sl_referenceDocRemoved(Document* d) {
    if (d != NULL && refObj != NULL && refObj->getDocument() == d) {
        onReferenceRemoved();
    }
}

// when reference obj removed from its document
void AssemblyModel::sl_referenceObjRemoved(GObject* o) {
    if (refObj != NULL && refObj == o) {
        onReferenceRemoved();
    }
}

// when reference doc added to project
void AssemblyModel::sl_referenceDocAdded(Document * d) {
    SAFE_POINT(d, "Reference document is NULL!", );

    if (refObj == NULL && !assembly.referenceId.isEmpty()) {
        U2OpStatusImpl status;
        U2CrossDatabaseReference ref = dbiHandle.dbi->getCrossDatabaseReferenceDbi()->getCrossReference(assembly.referenceId, status);
        SAFE_POINT_OP(status,);
        
        if (ref.dataRef.dbiRef.dbiId == d->getURLString()) {
            if (!d->isLoaded()) {
                startLoadReferenceTask(new LoadUnloadedDocumentTask(d));
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
        if (!loadingReference) {
            sl_referenceLoaded();
        }
    } else { // refDoc unloaded
        unsetReference();
        emit si_referenceChanged();
    }
}

// document is loaded and in the project -> create dbi handle and set reference
void AssemblyModel::sl_referenceLoaded() {
    U2OpStatusImpl os;
    U2CrossDatabaseReference ref = dbiHandle.dbi->getCrossDatabaseReferenceDbi()->getCrossReference(assembly.referenceId, os);
    refObj = NULL;
    Document* refDoc = AppContext::getProject()->findDocumentByURL(ref.dataRef.dbiRef.dbiId);
    if (refDoc != NULL) {
        refObj = qobject_cast<U2SequenceObject*>(refDoc->findGObjectByName(ref.dataRef.entityId.constData()));
    }

    loadingReference = false;
    if (refObj == NULL) {
        sl_unassociateReference();
    } else {
        emit si_referenceChanged();
    }

    connect(refDoc, SIGNAL(si_objectRemoved(GObject*)), SLOT(sl_referenceObjRemoved(GObject*)));
}

bool AssemblyModel::hasReference() const {
    return refObj != NULL;
}

bool AssemblyModel::referenceAssociated() const {
    return !assembly.referenceId.isEmpty();
}

void AssemblyModel::setReference(U2SequenceObject* seqObj) {
    refObj = seqObj;
    emit si_referenceChanged();
}

QByteArray AssemblyModel::getReferenceRegion(const U2Region& region, U2OpStatus& os) {
    SAFE_POINT(refObj, "Reference document is not ready!", QByteArray());
    return refObj->getSequenceData(region);
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

qint64 AssemblyModel::getReadsNumber(U2OpStatus & os) {
    if(cachedReadsNumber == NO_VAL) {
        U2AttributeDbi * attributeDbi = dbiHandle.dbi->getAttributeDbi();
        //U2OpStatusImpl os;
        static const QByteArray READS_COUNT_ATTRIBUTE_NAME("count_reads_attribute");
        if(attributeDbi != NULL) {
            U2IntegerAttribute attr = U2AttributeUtils::findIntegerAttribute(attributeDbi, assembly.id, READS_COUNT_ATTRIBUTE_NAME, os);
            LOG_OP(os);
            // If attribute found...
            if(attr.hasValidId()) {
                // ...check its version...
                if(attr.version == assembly.version) {
                    cachedReadsNumber = attr.value;
                } else {
                    // ...and remove if it's obsolete
                    U2AttributeUtils::removeAttribute(attributeDbi, attr.id, os);
                    LOG_OP(os);
                }
            }
        }
        if(cachedReadsNumber == NO_VAL) {
            // if could not get value from attribute, recompute the value...
            cachedReadsNumber = assemblyDbi->countReads(assembly.id, U2_REGION_MAX, os);
            LOG_OP(os);
            if(! os.isCoR()) {
                // ...and store it in a new attribure
                U2IntegerAttribute attr;
                U2AttributeUtils::init(attr, assembly, READS_COUNT_ATTRIBUTE_NAME);
                attr.value = cachedReadsNumber;
                attributeDbi->createIntegerAttribute(attr, os);
            }
        }
    }
    return cachedReadsNumber;
}

bool AssemblyModel::hasReads(U2OpStatus & os) {
    return getReadsNumber(os) != 0;
}


QList<U2AssemblyRead> AssemblyModel::findMateReads(U2AssemblyRead read, U2OpStatus& os) {
    QList<U2AssemblyRead> result;

    // don't even try to search if flag Fragmented is not set
    if(! ReadFlagsUtils::isPairedRead(read->flags)) {
        return result;
    }

    std::auto_ptr<U2DbiIterator<U2AssemblyRead> > it(assemblyDbi->getReadsByName(assembly.id, read->name, os));
    CHECK_OP(os, result);

    while (it->hasNext()) {
        U2AssemblyRead r = it->next();
        if(r->id != read->id) {
            result << r;
        }
    }
    return result;
}

QByteArray AssemblyModel::getReferenceSpecies(U2OpStatus & os) {
    if(!speciesRetrieved) {
        speciesRetrieved = true;
        U2AttributeDbi * attributeDbi = dbiHandle.dbi->getAttributeDbi();
        static const QByteArray SPECIES_ATTRIBUTE_NAME("reference_species_attribute");
        if (attributeDbi != NULL) {
            U2ByteArrayAttribute attr = U2AttributeUtils::findByteArrayAttribute(attributeDbi, assembly.id, SPECIES_ATTRIBUTE_NAME, os);
            if(attr.hasValidId()) {
                referenceSpecies = attr.value;
            }
        }
    }
    return referenceSpecies;
}

QString AssemblyModel::getReferenceUri(U2OpStatus & os) {
    if(!uriRetrieved) {
        uriRetrieved = true;
        U2AttributeDbi * attributeDbi = dbiHandle.dbi->getAttributeDbi();
        static const QByteArray URI_ATTRIBUTE_NAME("reference_uri_attribute");
        if(attributeDbi != NULL) {
            U2StringAttribute attr = U2AttributeUtils::findStringAttribute(attributeDbi, assembly.id, URI_ATTRIBUTE_NAME, os);
            if(attr.hasValidId()) {
                referenceUri = attr.value;
            }
        }
    }
    return referenceUri;
}

void AssemblyModel::sl_unassociateReference() {
    if(!assembly.referenceId.isEmpty()) {
        assembly.referenceId.clear();
        U2OpStatusImpl status;
        assemblyDbi->updateAssemblyObject(assembly, status);
        LOG_OP(status);
        unsetReference();
        emit si_referenceChanged();
    }
}

} // U2
