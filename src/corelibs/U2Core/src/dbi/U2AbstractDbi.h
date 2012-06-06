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

/**
    Default DBI implementations
*/

#ifndef _U2_ABSTRACT_DBI_H_
#define _U2_ABSTRACT_DBI_H_

#include <U2Core/U2DbiUtils.h>

#include <U2Core/U2FeatureDbi.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2CrossDatabaseReferenceDbi.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2VariantDbi.h>


namespace U2 {

/** Default (empty) implementation for optional DBI methods */
class U2AbstractDbi : public U2Dbi {
protected:
    U2AbstractDbi(const U2DbiFactoryId& fid) {
        state = U2DbiState_Void;
        factoryId = fid;
    }

public:    
    virtual bool flush(U2OpStatus&) {return true;}

    virtual U2DbiState getState() const {return state;}

    virtual U2DbiId getDbiId() const {return dbiId;}

    virtual U2DbiFactoryId getFactoryId() const  {return factoryId;}

    virtual const QSet<U2DbiFeature>& getFeatures() const {return features;}

    virtual QHash<QString, QString> getInitProperties() const {return initProperties;}

    virtual QHash<QString, QString> getDbiMetaInfo(U2OpStatus&) {return metaInfo;}

    virtual U2SequenceDbi* getSequenceDbi() {return NULL;}

    virtual U2FeatureDbi* getFeatureDbi() {return NULL;}

    virtual U2MsaDbi* getMsaDbi() {return NULL;}

    virtual U2AssemblyDbi* getAssemblyDbi() {return NULL;}

    virtual U2AttributeDbi* getAttributeDbi()  {return NULL;}
    
    virtual U2ObjectDbi* getObjectDbi()  {return NULL;}

    virtual U2VariantDbi* getVariantDbi()  {return NULL;}
    
    virtual U2CrossDatabaseReferenceDbi* getCrossDatabaseReferenceDbi()  {return NULL;}
    
    virtual U2DataType getEntityTypeById(const U2DataId&) const {return U2Type::Unknown;}

    virtual QString getProperty(const QString&, const QString& defaultValue, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_ReadProperties, this, os);
        return defaultValue;
    }

    virtual void setProperty(const QString&, const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteProperties, this, os);
    }

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

public:
    virtual void removeObject(const U2DataId&, const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_RemoveObjects, getRootDbi(), os);
    }

    virtual void removeObjects(const QList<U2DataId>&, const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_RemoveObjects, getRootDbi(), os);
    }

    virtual void createFolder(const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_ChangeFolders, getRootDbi(), os);
    }

    virtual void removeFolder(const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_ChangeFolders, getRootDbi(), os);
    }

    virtual void addObjectsToFolder(const QList<U2DataId>&, const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_ChangeFolders, getRootDbi(), os);
    }

    virtual void moveObjects(const QList<U2DataId>&, const QString&, const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_ChangeFolders, getRootDbi(), os);
    }
};

/** Default no-op implementation for write  methods of U2SequenceDbi */
class U2SimpleSequenceDbi: public U2SequenceDbi {
protected:
    U2SimpleSequenceDbi(U2Dbi* rootDbi) : U2SequenceDbi(rootDbi) {}

public:
    virtual void createSequenceObject(U2Sequence&, const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequence, getRootDbi(), os);
    }

    virtual void updateSequenceObject(U2Sequence& , U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequence, getRootDbi(), os);
    }

    virtual void updateSequenceData(const U2DataId&, const U2Region&, const QByteArray&, const QVariantMap &hints, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteSequence, getRootDbi(), os);
    }
};


/** Default no-op implementation for write  methods of U2MsaDbi */
class U2SimpleMsaDbi : public U2MsaDbi {
protected:
    U2SimpleMsaDbi(U2Dbi* rootDbi) : U2MsaDbi(rootDbi) {}

public:
    virtual void createMsaObject(U2Msa&, const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteMsa, getRootDbi(), os);
    }
    
    virtual void removeSequences(U2Msa&, const QList<U2DataId> , U2OpStatus &os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteMsa, getRootDbi(), os);
    }

    virtual void addSequences(U2Msa&, const QList<U2MsaRow>&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteMsa, getRootDbi(), os);
    }
};


/** Default no-op implementation for write  methods of U2AssemblyDbi */
class U2SimpleAssemblyDbi: public U2AssemblyDbi{
protected:
    U2SimpleAssemblyDbi(U2Dbi* rootDbi) : U2AssemblyDbi(rootDbi) {}

public:
    virtual void createAssemblyObject(U2Assembly&, const QString&,  U2DbiIterator<U2AssemblyRead>*, U2AssemblyReadsImportInfo&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAssembly, getRootDbi(), os);
    }

    virtual void updateAssemblyObject(U2Assembly&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAssembly, getRootDbi(), os);
    }

    virtual void removeReads(const U2DataId&, const QList<U2DataId>&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAssembly, getRootDbi(), os);
    }

    virtual void addReads(const U2DataId&, U2DbiIterator<U2AssemblyRead>*, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAssembly, getRootDbi(), os);
    }

    virtual void pack(const U2DataId&, U2AssemblyPackStat&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_AssemblyReadsPacking, getRootDbi(), os);
    }

    virtual void calculateCoverage(const U2DataId&, const U2Region&, U2AssemblyCoverageStat&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_AssemblyCoverageStat, getRootDbi(), os);
    }
};

/** Default no-op implementation for write  methods of U2FeatureDbi */
class U2SimpleFeatureDbi : public U2FeatureDbi {
protected:
    U2SimpleFeatureDbi(U2Dbi* rootDbi) : U2FeatureDbi(rootDbi) {}

public:
    virtual void createFeature(U2Feature&, const QList<U2FeatureKey>&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteFeatures, getRootDbi(), os);
    }

    virtual void addKey(const U2DataId&, const U2FeatureKey&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteFeatures, getRootDbi(), os);
    }

    virtual void removeAllKeys(const U2DataId&, const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteFeatures, getRootDbi(), os);
    }

    virtual void removeAllKeys(const U2DataId&, const U2FeatureKey&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteFeatures, getRootDbi(), os);
    }

    virtual void updateLocation(const U2DataId&, const U2FeatureLocation&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteFeatures, getRootDbi(), os);
    }

    virtual void removeFeature(const U2DataId&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteFeatures, getRootDbi(), os);
    }

};

/** Default no-op implementation for write  methods of U2AttributeDbi */
class U2SimpleAttributeDbi: public U2AttributeDbi {
protected:
    U2SimpleAttributeDbi(U2Dbi* rootDbi) : U2AttributeDbi(rootDbi) {}

public:    
    virtual void removeAttributes(const QList<U2DataId>&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi(), os);
    }

    virtual void removeObjectAttributes(const U2DataId&, U2OpStatus& os)  {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi(), os);
    }
        
    virtual void createIntegerAttribute(U2IntegerAttribute&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi(), os);
    }

    virtual void createRealAttribute(U2RealAttribute&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi(), os);
    }

    virtual void createStringAttribute(U2StringAttribute&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi(), os);
    }

    virtual void createByteArrayAttribute(U2ByteArrayAttribute&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteAttributes, getRootDbi(), os);
    }
};

class U2SimpleCrossDatabaseReferenceDbi: public U2CrossDatabaseReferenceDbi {
public:
    virtual U2CrossDatabaseReference getCrossReference(const U2DataId&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteCrossDatabaseReferences, getRootDbi(), os);
        return U2CrossDatabaseReference();
    }

    virtual void updateCrossReference(const U2CrossDatabaseReference&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteCrossDatabaseReferences, getRootDbi(), os);
    }
};


/** Default no-op implementation for write  methods of U2VariantDbi */
class U2SimpleVariantDbi: public U2VariantDbi {
protected:
    U2SimpleVariantDbi(U2Dbi* rootDbi) : U2VariantDbi(rootDbi) {}

public:
    virtual void createVariantTrack(U2VariantTrack& , U2DbiIterator<U2Variant>* , const QString&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteVariant, getRootDbi(), os); 
    }
 
    virtual void updateVariantTrack(const U2VariantTrack&, U2OpStatus& os) {
        U2DbiUtils::logNotSupported(U2DbiFeature_WriteVariant, getRootDbi(), os); 
    }
};



}//namespace

#endif
