#ifndef _U2_ASN_PARSER_TESTS_H_
#define _U2_ASN_PARSER_TESTS_H_

#include <U2Test/XMLTestUtils.h>

#include <U2Core/IOAdapter.h>

#include <QtXml/QDomElement>

namespace U2 {

class AsnNode;

class GTest_LoadAsnTree : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_LoadAsnTree, "load-asn-tree");

    ReportResult report();

    virtual void cleanup();

private:
    QString         asnTreeContextName;
    bool            contextAdded;
    AsnNode*        rootElem;
};

class GTest_FindFirstNodeByName : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_FindFirstNodeByName, "find-first-node-by-name");

    ReportResult report();
    void cleanup();
private:
    QString rootContextName;
    QString nodeContextName;
    QString nodeName;
    bool contextAdded;

};

class GTest_CheckNodeType : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CheckNodeType, "check-node-type");

    ReportResult report();
private:
    QString nodeContextName;
    QString nodeTypeName;
};


class GTest_CheckNodeValue : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CheckNodeValue, "check-node-value");

    ReportResult report();
private:
    QString nodeContextName;
    QString nodeValue;
};


class GTest_CheckNodeChildrenCount : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CheckNodeChildrenCount, "check-node-children-count");
    ReportResult report();
private:
    QString nodeContextName;
    int numChildren;
};


class AsnParserTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

}//namespace

#endif // _U2_ASN_PARSER_TESTS_H_
