#include "AttributeDbiTest.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2SqlHelpers.h>

#include <QtCore/QDir>


namespace U2 {

static const QString ATTRIBUTE_NAMES = "attribute_names";

static const QString OBJECT_ATTRIBUTES = "object_attributes";

static const QString OBJECT_ATTRIBUTES_NAME = "object_attributes_name";

static const QString OBJECT_ATTRIBUTES_CHILD_ID = "object_attributes_child_id";

static APITestData createTestData() {
    APITestData d;
    
    d.addValue<QString>(BaseDbiTest::DB_URL, "attribute-dbi.ugenedb");

    // getAvailableAttributeNames
    {
        QStringList names;
        names << "int1" << "int2" << "real1" << "str1" << "str2" << "arr1";
        d.addList(ATTRIBUTE_NAMES, names);
    }

    const U2DataId& childId = SQLiteUtils::toU2DataId(2, U2Type::Assembly);
    
    // getObjectAttributes
    {
        ObjectAttributesTestData oat;
        oat.objId = SQLiteUtils::toU2DataId(1, U2Type::Sequence);

        {
	        U2IntegerAttribute attr;
	        attr.objectId = oat.objId;
	        attr.name = "int1";
	        attr.value = 5;
            attr.childId = childId;
            oat.intAttrs.append(attr);
        }

        {
            U2IntegerAttribute attr;
            attr.objectId = oat.objId;
            attr.name = "int2";
            attr.value = 3;
            attr.childId = childId;
            oat.intAttrs.append(attr);
        }

        {
            U2RealAttribute attr;
            attr.objectId = oat.objId;
            attr.name = "real1";
            attr.value = 2.7;
            attr.childId = childId;
            oat.realAttrs.append(attr);
        }

        {
            U2StringAttribute attr;
            attr.objectId = oat.objId;
            attr.name = "str1";
            attr.value = "some string";
            oat.stringAttrs.append(attr);
        }

        {
	        U2StringAttribute attr;
	        attr.objectId = oat.objId;
	        attr.name = "str2";
	        attr.value = "other string";
            oat.stringAttrs.append(attr);
        }

        {
	        U2ByteArrayAttribute attr;
	        attr.objectId = oat.objId;
	        attr.name = "arr1";
	        attr.value = "some array";
            oat.byteArrAttrs.append(attr);
        }

        d.addValue(OBJECT_ATTRIBUTES, oat);
    }

    // getObjectAttributesByName
    d.addValue<QString>(OBJECT_ATTRIBUTES_NAME, "int2");

    // getObjectPairAttributes
    {
	    d.addValue<U2DataId>(OBJECT_ATTRIBUTES_CHILD_ID, childId);
    }

    return d;
}

static bool compareAttributesBase(const U2Attribute& attr1, const U2Attribute& attr2) {
    /*if (attr1.id != attr2.id) {
        return false;
    }*/
    if (attr1.objectId != attr2.objectId) {
        return false;
    }
    if (attr1.childId != attr2.childId) {
        return false;
    }
    if (attr1.version != attr2.version) {
        return false;
    }
    if (attr1.name != attr2.name) {
        return false;
    }
    return true;
}

template<class A>
static bool compareAttributes(const A& attr1, const A& attr2) {
    if (attr1.value != attr2.value) {
        return false;
    }
    return compareAttributesBase(attr1, attr2);
}

template<class T, typename Compare>
bool removeOne(QList<T>& list, const T& el, Compare compare) {
    QMutableListIterator<T> iter(list);
    while (iter.hasNext()) {
        const T& current = iter.next();
        if (compare(current, el)) {
            iter.remove();
            return true;
        }
    }
    return false;
}

template<class T>
void filterAttributesByName(QList<T>& attrs, const QString& name) {
    if (!name.isEmpty()) {
        QMutableListIterator<T> iter(attrs);
        while (iter.hasNext()) {
            const T& current = iter.next();
            if (current.name != name) {
                iter.remove();
            }
        }
    }
}

template<class T>
void filterAttributesByChildId(QList<T>& attrs, const U2DataId& childId) {
    QMutableListIterator<T> iter(attrs);
    while (iter.hasNext()) {
        const T& current = iter.next();
        if (current.childId != childId) {
            iter.remove();
        }
    }
}

void AttributeDbiTest::SetUp() {
    ASSERT_NO_FATAL_FAILURE(BaseDbiTest::SetUp());
    attributeDbi = dbi->getAttributeDbi();

    U2ObjectDbi* objectDbi = dbi->getObjectDbi();
    U2OpStatusImpl os;
    objects = objectDbi->getObjects("/", 0, U2_DBI_NO_LIMIT, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
}

TEST_P(AttributeDbiTest, getAvailableAttributeNames) {
    U2OpStatusImpl os;
    QStringList actual = this->attributeDbi->getAvailableAttributeNames(os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    QStringList expected = testData.getList<QString>(ATTRIBUTE_NAMES);

    ASSERT_EQ(actual.size(), expected.size());
    foreach(const QString& name, expected) {
        ASSERT_TRUE(actual.removeOne(name));
    }
    ASSERT_EQ(actual.size(), 0);
}

void testAttributesMatch(U2AttributeDbi* attributeDbi,
                         QList<U2IntegerAttribute>& expectedInt,
                         QList<U2RealAttribute>& expectedReal,
                         QList<U2StringAttribute>& expectedString,
                         QList<U2ByteArrayAttribute>& expectedArray,
                         const QList<U2DataId>& attributes)
{
    foreach(const U2DataId& attrId, attributes) {
        U2DataType type = SQLiteUtils::toType(attrId);
        ASSERT_TRUE(U2Type::isAttributeType(type));

        U2OpStatusImpl os;

        if (type == U2Type::AttributeInteger) {
            U2IntegerAttribute attr = attributeDbi->getIntegerAttribute(attrId, os);
            ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
            ASSERT_TRUE(removeOne(expectedInt, attr, compareAttributes<U2IntegerAttribute>));
        } else if (type == U2Type::AttributeReal) {
            U2RealAttribute attr = attributeDbi->getRealAttribute(attrId, os);
            ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
            ASSERT_TRUE(removeOne(expectedReal, attr, compareAttributes<U2RealAttribute>));
        } else if (type == U2Type::AttributeString) {
            U2StringAttribute attr = attributeDbi->getStringAttribute(attrId, os);
            ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
            ASSERT_TRUE(removeOne(expectedString, attr, compareAttributes<U2StringAttribute>));
        } else { // AttributeByteArray
            U2ByteArrayAttribute attr = attributeDbi->getByteArrayAttribute(attrId, os);
            ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
            ASSERT_TRUE(removeOne(expectedArray, attr, compareAttributes<U2ByteArrayAttribute>));
        }
    }

    ASSERT_EQ(expectedInt.size(), 0);
    ASSERT_EQ(expectedReal.size(), 0);
    ASSERT_EQ(expectedString.size(), 0);
    ASSERT_EQ(expectedArray.size(), 0);
}

void getObjectAttrsTest(U2AttributeDbi* attributeDbi, const APITestData& testData, bool filterByName, bool filterByChild) {
    const ObjectAttributesTestData& attrsData = testData.getValue<ObjectAttributesTestData>(OBJECT_ATTRIBUTES);

    const U2DataId& objectId = attrsData.objId;

    QList<U2IntegerAttribute> attrsInt = attrsData.intAttrs;
    QList<U2RealAttribute> attrsReal = attrsData.realAttrs;
    QList<U2StringAttribute> attrsString = attrsData.stringAttrs;
    QList<U2ByteArrayAttribute> attrsByteArray = attrsData.byteArrAttrs;

    const U2DataId& childId = testData.getValue<U2DataId>(OBJECT_ATTRIBUTES_CHILD_ID);
    QString name;

    if (filterByName) {
        name = testData.getValue<QString>(OBJECT_ATTRIBUTES_NAME);
        filterAttributesByName(attrsInt, name);
        filterAttributesByName(attrsReal, name);
        filterAttributesByName(attrsString, name);
        filterAttributesByName(attrsByteArray, name);
    }

    QList<U2DataId> attributes;
    U2OpStatusImpl os;
    if (filterByChild) {
        filterAttributesByChildId(attrsInt, childId);
        filterAttributesByChildId(attrsReal, childId);
        filterAttributesByChildId(attrsString, childId);
        filterAttributesByChildId(attrsByteArray, childId);
        attributes = attributeDbi->getObjectPairAttributes(objectId, childId, name, os);
    } else {
        attributes = attributeDbi->getObjectAttributes(objectId, name, os);
    }
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    testAttributesMatch(attributeDbi, attrsInt, attrsReal, attrsString, attrsByteArray, attributes);
}

TEST_P(AttributeDbiTest, getObjectAttributes) {
    getObjectAttrsTest(this->attributeDbi, testData, false, false);
}

TEST_P(AttributeDbiTest, getObjectAttributesByName) {
    getObjectAttrsTest(this->attributeDbi, testData, true, false);
}

TEST_P(AttributeDbiTest, getObjectPairAttributes) {
    getObjectAttrsTest(this->attributeDbi, testData, false, true);
}

TEST_P(AttributeDbiTest, getObjectPairAttributesByName) {
    getObjectAttrsTest(this->attributeDbi, testData, true, true);
}

TEST_P(AttributeDbiTest, removeAttributes) {
    const U2DataId& objectId = objects.first();
    U2OpStatusImpl os;
    const QList<U2DataId>& attrs = this->attributeDbi->getObjectAttributes(objectId, "", os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();

    {
        U2OpStatusImpl os;
        this->attributeDbi->removeAttributes(attrs, os);
        ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    }
    {
        U2OpStatusImpl os;
        const QList<U2DataId>& attrs = this->attributeDbi->getObjectAttributes(objectId, "", os);
        ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
        ASSERT_TRUE(attrs.isEmpty());
    }
}

TEST_P(AttributeDbiTest, removeObjectAttributes) {
    const U2DataId& objectId = objects.first();
    {
	    U2OpStatusImpl os;
	    this->attributeDbi->removeObjectAttributes(objectId, os);
	    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    }
    U2OpStatusImpl os;
    const QList<U2DataId>& attrs = this->attributeDbi->getObjectAttributes(objectId, "", os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_TRUE(attrs.isEmpty());
}

TEST_P(AttributeDbiTest, IntegerAttribute) {
    U2IntegerAttribute attr;
    attr.objectId = this->objects.first();
    attr.value = 46;

    {
        U2OpStatusImpl os;
        this->attributeDbi->createIntegerAttribute(attr, os);
        ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    }

    U2OpStatusImpl os;
    U2IntegerAttribute actual = this->attributeDbi->getIntegerAttribute(attr.id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_TRUE(compareAttributes(actual, attr));
}

TEST_P(AttributeDbiTest, RealAttribute) {
    U2RealAttribute attr;
    attr.objectId = this->objects.first();
    attr.value = 46.46;

    {
	    U2OpStatusImpl os;
	    this->attributeDbi->createRealAttribute(attr, os);
	    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    }

    U2OpStatusImpl os;
    U2RealAttribute actual = this->attributeDbi->getRealAttribute(attr.id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_TRUE(compareAttributes(actual, attr));
}

TEST_P(AttributeDbiTest, StringAttribute) {
    U2StringAttribute attr;
    attr.objectId = this->objects.first();
    attr.value = "some string";

    {
        U2OpStatusImpl os;
        this->attributeDbi->createStringAttribute(attr, os);
        ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    }

    U2OpStatusImpl os;
    U2StringAttribute actual = this->attributeDbi->getStringAttribute(attr.id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_TRUE(compareAttributes(actual, attr));
}

TEST_P(AttributeDbiTest, ByteArrayAttribute) {
    U2ByteArrayAttribute attr;
    attr.objectId = this->objects.first();
    attr.value = "some array";

    {
        U2OpStatusImpl os;
        this->attributeDbi->createByteArrayAttribute(attr, os);
        ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    }

    U2OpStatusImpl os;
    U2ByteArrayAttribute actual = this->attributeDbi->getByteArrayAttribute(attr.id, os);
    ASSERT_FALSE(os.hasError()) << os.getError().toStdString();
    ASSERT_TRUE(compareAttributes(actual, attr));
}

INSTANTIATE_TEST_CASE_P(
                        AttributeDbiTestInstance,
                        AttributeDbiTest,
                        ::testing::Values(createTestData()));

} //namespace
