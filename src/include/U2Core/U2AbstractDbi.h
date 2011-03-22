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

/**
    Default DBI implementations
*/

#ifndef _U2_ABSTRACT_DBI_H_
#define _U2_ABSTRACT_DBI_H_

#include <U2Core/U2DbiUtils.h>


namespace U2 {

    /** Default (empty) implementation for optional DBI methods */
class U2AbstractDbi : public U2Dbi {
public:
    U2AbstractDbi(const U2DbiFactoryId& fid) {
        state = U2DbiState_Void;
        factoryId = fid;
    }
    
    virtual bool flush(U2OpStatus& os) {return true;}

    virtual U2DbiState getState() const {return state;}

    virtual U2DbiId getDbiId() const {return dbiId;}

    virtual U2DbiFactoryId getFactoryId() const  {return factoryId;}

    virtual const QSet<U2DbiFeature>& getFeatures() const {return features;}

    virtual QHash<QString, QString> getInitProperties() const {return initProperties;}

    virtual QHash<QString, QString> getDbiMetaInfo(U2OpStatus&) {return metaInfo;}

    virtual U2SequenceDbi* getSequenceDbi() {return NULL;}

    virtual U2AnnotationDbi* getAnnotationRDbi() {return NULL;}

    virtual U2MsaDbi* getMsaDbi() {return NULL;}

    virtual U2AssemblyDbi* getAssemblyDbi() {return NULL;}

    virtual U2AttributeDbi* getAttributeDbi()  {return NULL;}

protected:
    U2DbiState                  state;
    U2DbiId                     dbiId;
    U2DbiFactoryId              factoryId;
    QSet<U2DbiFeature>          features;
    QHash<QString, QString>     initProperties;
    QHash<QString, QString>     metaInfo;
};


/** Default no-op implementation for write  methods of U2ObjectDbi */
class U2SimpleObjectDbi : public U2ObjectDbi {
protected:
    U2SimpleObjectDbi(U2Dbi* rootDbi) : U2ObjectDbi(rootDbi) {}

    virtual void removeObject(const U2DataId& dataId, const QString& folder, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_RemoveObjects, getRootDbi());
    }

    virtual void removeObjects(const QList<U2DataId>& dataIds, const QString& folder, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_RemoveObjects, getRootDbi());
    }

    virtual void createFolder(const QString& path, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_ChangeFolders, getRootDbi());
    }

    virtual void removeFolder(const QString& folder, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_ChangeFolders, getRootDbi());
    }

    virtual void addObjectsToFolder(const QList<U2DataId>& objectIds, const QString& toFolder, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_ChangeFolders, getRootDbi());
    }

    virtual void moveObjects(const QList<U2DataId>& objectIds, const QString& fromFolder, const QString& toFolder, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_ChangeFolders, getRootDbi());
    }
};

/** Default no-op implementation for write  methods of U2SequenceDbi */
class U2SimpleSequenceDbi: public U2SequenceDbi {
protected:
    U2SimpleSequenceDbi(U2Dbi* rootDbi) : U2SequenceDbi(rootDbi) {}

    virtual void createSequenceObject(U2Sequence& sequence, const QString& folder, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequence, getRootDbi());
    }

    virtual void updateSequenceData(const U2DataId& sequenceId, const U2Region& regionToReplace, const QByteArray& dataToInsert, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequence, getRootDbi());
    }
};


/** Default no-op implementation for write  methods of U2MsaDbi */
class U2SimpleMsaDbi: public U2MsaDbi{
protected:
    U2SimpleMsaDbi(U2Dbi* rootDbi) : U2MsaDbi(rootDbi) {}
    
    virtual void createMsaObject(U2Msa& msa, const QString& folder, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteMsa, getRootDbi());
    }
    
    virtual void removeSequences(U2Msa& msa, const QList<U2DataId> sequenceIds, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteMsa, getRootDbi());
    }

    virtual void addSequences(U2Msa& msa, const QList<U2MsaRow>& rows, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteMsa, getRootDbi());
    }
};


/** Default no-op implementation for write  methods of U2AssemblyDbi */
class U2SimpleAssemblyDbi: public U2AssemblyDbi{
protected:
    U2SimpleAssemblyDbi(U2Dbi* rootDbi) : U2AssemblyDbi(rootDbi) {}

    virtual void createAssemblyObject(U2Assembly& assembly, const QString& folder, U2AssemblyReadsIterator* it, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAssembly, getRootDbi());
    }

    virtual void removeReads(const U2DataId& assemblyId, const QList<U2DataId>& rowIds, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAssembly, getRootDbi());
    }

    virtual void addReads(const U2DataId& assemblyId, QList<U2AssemblyRead>& rows, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAssembly, getRootDbi());
    }

    virtual void pack(const U2DataId& assemblyId, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_AssemblyReadsPacking, getRootDbi());
    }
};

/** Default no-op implementation for write  methods of U2AnnotationDbi */
class U2SimpleAnnotationDbi: public U2AnnotationDbi {
protected:
    U2SimpleAnnotationDbi(U2Dbi* rootDbi) : U2AnnotationDbi(rootDbi) {}

    virtual void createAnnotation(U2Annotation& a, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequenceAnnotations, getRootDbi());
    }

    virtual void createAnnotations(QList<U2Annotation>& annotations, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequenceAnnotations, getRootDbi());
    }
    
    virtual void removeAnnotation(const U2DataId& annotationId, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequenceAnnotations, getRootDbi());
    }

    virtual void removeAnnotations(const QList<U2DataId>& annotationIds, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequenceAnnotations, getRootDbi());
    }
    
    virtual void updateLocation(const U2DataId& annotationId, const U2Location& location, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequenceAnnotations, getRootDbi()); 
    }
    
    virtual void updateName(const U2DataId& annotationId, const QString& newName, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequenceAnnotations, getRootDbi()); 
    }
    
    virtual void createQualifier(const U2DataId& annotationId, const U2Qualifier& q, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequenceAnnotations, getRootDbi()); 
    }
    
    virtual void removeQualifier(const U2DataId& annotationId, const U2Qualifier& q, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequenceAnnotations, getRootDbi());
    }

    virtual void addToGroup(const U2DataId& annotationId, const QString& group, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequenceAnnotations, getRootDbi());
    }
    
    virtual void removeFromGroup(const U2DataId& annotationId, const QString& group, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequenceAnnotations, getRootDbi());
    }
};

/** Default no-op implementation for write  methods of U2AttributeDbi */
class U2SimpleAttributeDbi: public U2AttributeDbi{
protected:
    U2SimpleAttributeDbi(U2Dbi* rootDbi) : U2AttributeDbi(rootDbi) {}
    
    virtual void removeAttribute(const U2DataId& attributeId, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi());
    }
    
    virtual void createInt32Attribute(U2Int32Attribute& a, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi());
    }

    virtual void createInt64Attribute(U2Int64Attribute& a, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi());
    }

    virtual void createReal64Attribute(U2Real64Attribute& a, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi());
    }

    virtual void createStringAttribute(U2StringAttribute& a, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi());
    }

    virtual void createByteArrayAttribute(U2ByteArrayAttribute& a, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi());
    }

    virtual void createDateTimeAttribute(U2DateTimeAttribute& a, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi());
    }

    virtual void createRangeInt32StatAttribute(U2RangeInt32StatAttribute& a, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi());
    }

    virtual void createRangeReal64StatAttribute(U2RangeReal64StatAttribute& a, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi());
    }
};


}//namespace

#endif
