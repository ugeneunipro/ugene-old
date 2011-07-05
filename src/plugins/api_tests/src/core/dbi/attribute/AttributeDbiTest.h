#ifndef _U2_ATTRIBUTE_DBI_TEST_H_
#define _U2_ATTRIBUTE_DBI_TEST_H_

#include <core/dbi/DbiTest.h>


namespace U2 {

class AttributeDbiTest : public BaseDbiTest {
protected:
    void SetUp();
protected:
    U2AttributeDbi* attributeDbi;
    QList<U2DataId> objects;
};

class ObjectAttributesTestData {
public:
    U2DataId objId;
    QList<U2IntegerAttribute> intAttrs;
    QList<U2RealAttribute> realAttrs;
    QList<U2StringAttribute> stringAttrs;
    QList<U2ByteArrayAttribute> byteArrAttrs;
};

} //namespace

Q_DECLARE_METATYPE(U2::U2IntegerAttribute);
Q_DECLARE_METATYPE(U2::U2RealAttribute);
Q_DECLARE_METATYPE(U2::U2StringAttribute);
Q_DECLARE_METATYPE(U2::U2ByteArrayAttribute);
Q_DECLARE_METATYPE(U2::ObjectAttributesTestData);

#endif
