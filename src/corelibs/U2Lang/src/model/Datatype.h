#ifndef _U2_WORKFLOW_DATATYPE_H_
#define _U2_WORKFLOW_DATATYPE_H_

#include <U2Lang/Descriptor.h>
#include <U2Core/IdRegistry.h>
#include <QtCore/QExplicitlySharedDataPointer>

namespace U2 {

class DataType;
typedef QExplicitlySharedDataPointer<DataType> DataTypePtr;

/**
 * represents a type for every data actor work with
 * all data got from user has some Datatype
 * all data passed from actor to actor has some Datatype
 */
class U2LANG_EXPORT DataType : public Descriptor, public QSharedData {
public:
    // there are 3 types of Datatype
    enum Kind {
        // simple types: string, int, bool, sequence, msa...
        Single,
        // complex types
        // used as types of ports
        // Data between actors passed as values of such complex types
        // list - list of values of some type
        List,
        // Map<Descriptor, DatatypePtr>
        Map
    }; // Kind
    
    DataType(const QString& id, const QString& name, const QString& desc);
    DataType(const Descriptor& d);
    virtual ~DataType() {}
    
    /*virtual bool equals(DataTypePtr t) const {return t == this;} */
    
    // default kind() is Single
    virtual Kind kind() const;
    inline bool isSingle() const {return kind() == Single;}
    inline bool isMap() const {return kind() == Map;}
    inline bool isList() const {return kind() == List;}
    
    // default: empty descriptor
    // in map type: corresponding descriptor
    // in list type: element type
    virtual DataTypePtr getDatatypeByDescriptor(const Descriptor& idd=Descriptor(QString())) const;
    
    // used only in map type: returns list of all types from map
    // default: empty list
    virtual QList<Descriptor> getAllDescriptors() const;
    
    // used only in map type
    // default: empty map
    virtual QMap<Descriptor, DataTypePtr> getDatatypesMap() const;
    
    // finds Descriptor identified with 'id' in list of Descriptors from getAllDescriptors function
    Descriptor getDatatypeDescriptor(const QString& id) const;
    
}; // DataType


/**
 * represents complex type. 
 * in fact, map of pairs (Descriptor, Datatype)
 * Usage: for passing many objects of different types (see IntegralBusType)
 */
class U2LANG_EXPORT MapDataType : public DataType {
public:
    MapDataType(const Descriptor& d, const QMap<Descriptor, DataTypePtr>& m);
    
    // reimplemented from Datatype
    virtual DataType::Kind kind() const;
    virtual DataTypePtr getDatatypeByDescriptor(const Descriptor& d) const;
    virtual QList<Descriptor> getAllDescriptors() const;
    virtual QMap<Descriptor, DataTypePtr> getDatatypesMap() const;
    
protected:
    // types map
    QMap<Descriptor, DataTypePtr> map;
    
}; // MapDataType

/**
 * represents complex type
 * list of elements of 1 type
 */
class U2LANG_EXPORT ListDataType : public DataType {
public:
    ListDataType(const Descriptor& d, DataTypePtr el);
    
    // reimplemented from Datatype
    virtual DataType::Kind kind() const;
    virtual DataTypePtr getDatatypeByDescriptor(const Descriptor& idd=Descriptor(QString())) const;
    
protected:
    // 
    DataTypePtr listElementDatatype;
    
}; // ListDataType

/**
 * standard registry for datatypes
 * we use it because we want to have 1 instance of each datatype
 * and place where we can get it
 * usage: see BaseTypes
 */
class DataTypeRegistry {
public:
    virtual ~DataTypeRegistry();
    
    virtual DataTypePtr getById(const QString& id) const;
    virtual bool registerEntry(DataTypePtr t);
    virtual DataTypePtr unregisterEntry(const QString& id);
    
    virtual QList<DataTypePtr> getAllEntries() const;
    virtual QList<QString> getAllIds() const;
    
protected:
    // standard map
    QMap<QString, DataTypePtr> registry;
    
}; // DataTypeRegistry

/**
 * abstract factory for creating values from String
 * used in cmdline: user pass values for attributes, we need convert them to qvariants
 * because attributes saves values in qvariant
 */
class DataTypeValueFactory {
public:
    DataTypeValueFactory() {}
    virtual ~DataTypeValueFactory() {}
    
    virtual QVariant getValueFromString( const QString & str, bool * ok = NULL ) const = 0;
    virtual QString getId() const = 0;
    
}; // DataTypeValueFactory

/**
 * Standard registry for DatatypeValueFactories
 */
typedef IdRegistry<DataTypeValueFactory> DataTypeValueFactoryRegistry;

} //namespace U2

#endif // _U2_WORKFLOW_DATATYPE_H_
