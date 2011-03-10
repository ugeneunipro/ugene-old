#ifndef __U2_BASE_TYPES_H_
#define __U2_BASE_TYPES_H_

#include <U2Lang/Datatype.h>

namespace U2 {

class U2LANG_EXPORT BaseTypes : public QObject {
    Q_OBJECT
public:
    static DataTypePtr DNA_SEQUENCE_TYPE();
    static DataTypePtr ANNOTATION_TABLE_LIST_TYPE();
    static DataTypePtr ANNOTATION_TABLE_TYPE();
    static DataTypePtr MULTIPLE_ALIGNMENT_TYPE();
    
    static DataTypePtr STRING_TYPE();
    static DataTypePtr BOOL_TYPE();
    static DataTypePtr NUM_TYPE();
    static DataTypePtr ANY_TYPE();
    
}; // BaseTypes

// value factories for core datatypes
/**
* DatatypeValueFactory realization for string type
*/
class StringTypeValueFactory : public DataTypeValueFactory {
public:
    virtual QVariant getValueFromString( const QString & str, bool * ok = NULL  ) const;
    virtual QString getId() const { return BaseTypes::STRING_TYPE()->getId(); }

}; // StringTypeValueFactory

/**
* DatatypeValueFactory realization for bool type
*/
class BoolTypeValueFactory : public DataTypeValueFactory {
public:
    static const QString TRUE_STR;
    static const QString FALSE_STR;
    static const QString YES_STR;
    static const QString NO_STR;
    static const QString ONE_STR;
    static const QString NIL_STR;

public:
    virtual QVariant getValueFromString( const QString & str, bool * ok = NULL  ) const;
    virtual QString getId() const { return BaseTypes::BOOL_TYPE()->getId(); }

}; // BoolTypeValueFactory

/**
* DatatypeValueFactory realization for num type
*/
class NumTypeValueFactory : public DataTypeValueFactory {
public:
    virtual QVariant getValueFromString( const QString & str, bool * ok = NULL ) const;
    virtual QString getId() const { return BaseTypes::NUM_TYPE()->getId(); }

}; // NumTypeValueFactory

}

#endif // __U2_BASE_TYPES_H_
