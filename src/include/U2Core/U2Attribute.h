#ifndef _U2_ATTRIBUTE_H_
#define _U2_ATTRIBUTE_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Region.h>

#include <QtCore/QDateTime>
#include <QtCore/QVector>

namespace U2 {


/**
    Attribute for any top-level object record.
    Some name/value are supported internally and must be tracked by the system on sequence modification
*/
class U2CORE_EXPORT U2Attribute : public U2Entity {
public:
    /** parent object id */
    U2DataId    objectId;

    /** Additional object id, optional */
    U2DataId    childId;

    /** parent object version this attribute is related to. If <=0 -> any is OK. */
    qint32      version;
    
    /* Name of the attribute */
    QString     name;
};

/** 32 bit signed integer attribute */
class U2CORE_EXPORT U2Int32Attribute : public U2Attribute {
public:
    qint32 value;
};

/** 64 bit signed integer attribute */
class U2CORE_EXPORT U2Int64Attribute : public U2Attribute {
public:
    qint64 value;
};

/** 64 bit real attribute */
class U2CORE_EXPORT U2Real64Attribute : public U2Attribute {
public:
    double value;
};

/** String attribute */
class U2CORE_EXPORT U2StringAttribute : public U2Attribute {
public:
    QString value;
};

/** Byte array attribute */
class U2CORE_EXPORT U2ByteArrayAttribute: public U2Attribute {
public:
    QByteArray value;
};

/** Date time attribute */
class U2CORE_EXPORT U2DateTimeAttribute: public U2Attribute {
public:
    QDateTime value;
};

/** 
    Base class for all range attributes: defines a range this attribute is contains info for
*/
class U2CORE_EXPORT U2RangeStatAttribute : public U2Attribute {
public:
    U2Region region;
};

/** Range attribute with 32-bit integers content. Defines 32-bit integer value for every point in the range */
class U2CORE_EXPORT U2RangeInt32StatAttribute: public U2RangeStatAttribute {
public:
    QVector<qint32> values;
};

/** Range attribute with 64-bit real content. Defines 64-bit real value for every point in the range */
class U2CORE_EXPORT U2RangeReal64StatAttribute: public U2RangeStatAttribute {
public:
    QVector<double> values;
};


} //namespace

#endif
