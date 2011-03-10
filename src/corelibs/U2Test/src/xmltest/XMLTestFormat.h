#ifndef _U2_XML_TEST_FORMAT_H_
#define _U2_XML_TEST_FORMAT_H_

#include "../GTest.h"

namespace U2 {

class XMLTestFormat;

class U2TEST_EXPORT XMLTestFactory {
public:
    XMLTestFactory(const QString& _tagName) : tagName(_tagName){}
    virtual ~XMLTestFactory();

    virtual GTest* createTest(XMLTestFormat* tf, const QString& testName, GTest* cp, const GTestEnvironment* env, 
                              const QList<GTest*>& subtasks, const QDomElement& el) = 0;

    const QString& getTagName() const {return tagName;}

private:
    QString tagName;
};

class U2TEST_EXPORT XMLTestFormat : public GTestFormat {
    Q_OBJECT
public:
    XMLTestFormat();
    virtual ~XMLTestFormat();
    
    virtual GTest* createTest(const QString& name, GTest* cp, const GTestEnvironment* env, const QByteArray& testData, QString& err);
    
    virtual GTest* createTest(const QString& name, GTest* cp, const GTestEnvironment* env, const QDomElement& el, QString& err);

    virtual bool registerTestFactory(XMLTestFactory* tf);

    virtual bool unregisterTestFactory(XMLTestFactory* tf);

private:
    void registerBuiltInFactories();
    QMap<QString, XMLTestFactory*> testFactories;
};

}//namespace

#endif
