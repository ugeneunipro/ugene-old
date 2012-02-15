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

#ifndef __U2_ASSEMBLY_BROWSER_MODEL_H__
#define __U2_ASSEMBLY_BROWSER_MODEL_H__

#include <QtCore/QPointer>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/GObject.h>

namespace U2 {

class Document;
class Task;
class U2SequenceObject;

class AssemblyModel : public QObject {
    Q_OBJECT
public:
    //TODO refactor 
    AssemblyModel(const DbiConnection& dbiConnection);
    ~AssemblyModel();
    
    bool isEmpty() const;

    QList<U2AssemblyRead> getReadsFromAssembly(const U2Region & r, qint64 minRow, qint64 maxRow, U2OpStatus & os);

    void calculateCoverageStat(const U2Region & r, U2AssemblyCoverageStat & stat, U2OpStatus & os);

    const U2AssemblyCoverageStat &getCoverageStat(U2OpStatus & os);
    
    U2Region getGlobalRegion();

    qint64 getModelLength(U2OpStatus & os);

    QByteArray getReferenceMd5(U2OpStatus & os);
    
    QByteArray getReferenceSpecies(U2OpStatus & os);
    
    QString getReferenceUri(U2OpStatus & os);
    
    qint64 getModelHeight(U2OpStatus & os);

    void setAssembly(U2AssemblyDbi * dbi, const U2Assembly & assm);

    bool hasReference() const;
    bool referenceAssociated()const;
    
    void setReference(U2SequenceObject* seqObj);

    QByteArray getReferenceRegion(const U2Region& region, U2OpStatus& os);

    const DbiConnection& getDbiConnection() const {return dbiHandle;}
    
    void associateWithReference(const U2CrossDatabaseReference & ref);
    
    bool isLoadingReference()const { return loadingReference; }
    
    qint64 getReadsNumber(U2OpStatus & os);
    bool hasReads(U2OpStatus & os);

    QList<U2AssemblyRead> findMateReads(U2AssemblyRead read, U2OpStatus& os);
    
private:
    /**
        Unsets reference only for current session (e.g. when ref doc is closed),
        association in ugenedb still exists
    */
    void unsetReference();
    void startLoadReferenceTask(Task * t);
    Task * createLoadReferenceAndAddToProjectTask(const U2CrossDatabaseReference& ref);
    void onReferenceRemoved();
    
signals:
    void si_referenceChanged();
    
private slots:
    void sl_referenceLoaded();
    void sl_referenceLoadingFailed();
    void sl_referenceDocLoadedStateChanged();
    void sl_referenceDocRemoved(Document*);
    void sl_referenceDocAdded(Document *);
    void sl_referenceObjRemoved(GObject* o);
    void sl_unassociateReference();
    
private:
    const static qint64 NO_VAL = -1;
    //TODO: track model changes and invalidate caches accordingly
    qint64 cachedModelLength;
    qint64 cachedModelHeight;

    
    U2Assembly assembly;
    U2AssemblyDbi * assemblyDbi;
    DbiConnection dbiHandle;

    bool loadingReference;
    U2SequenceObject* refObj;

    QByteArray referenceMd5;
    bool md5Retrieved;
    
    qint64 cachedReadsNumber;
    
    QByteArray referenceSpecies;
    bool speciesRetrieved;

    QString referenceUri;
    bool uriRetrieved;

    U2AssemblyCoverageStat cachedCoverageStat;

    QMutex mutex;
}; // AssemblyModel

} // U2

#endif // __U2_ASSEMBLY_BROWSER_MODEL_H__
