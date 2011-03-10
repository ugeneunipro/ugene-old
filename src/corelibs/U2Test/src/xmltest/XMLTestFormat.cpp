#include "XMLTestFormat.h"

#define TEST_FACTORIES_AUTO_CLEANUP

#ifdef TEST_FACTORIES_AUTO_CLEANUP
    #include <U2Core/AppContext.h>
    #include <U2Test/GTestFrameworkComponents.h>
#endif

#include <U2Test/XMLTestUtils.h>

namespace U2 {

XMLTestFactory::~XMLTestFactory() {
#ifdef TEST_FACTORIES_AUTO_CLEANUP
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    if (tfr!=NULL) {
        XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
        if (xmlTestFormat!=NULL) {
            xmlTestFormat->unregisterTestFactory(this);
        }
    }
#endif
}



XMLTestFormat::XMLTestFormat() : GTestFormat("XML")
{
    registerBuiltInFactories();
}

XMLTestFormat::~XMLTestFormat() {
    foreach(XMLTestFactory* f, testFactories.values()) {
        delete f;
    }
}

GTest* XMLTestFormat::createTest(const QString& name, GTest* cp, const GTestEnvironment* env, const QByteArray& testData, QString& err) {
    QDomDocument doc;
    int line = 0;
    int col = 0;
    bool res = doc.setContent(testData, &err, &line, &col);
    if (!res) {
        err = QString("error_reading_test: ") + err;
        err+=QString(" line: %1 col: %2").arg(QString::number(line)).arg(QString::number(col));
        return NULL;
    }
    if (doc.doctype().name()!="UGENE_TEST_FRAMEWORK_TEST") {
        err = QString("not_a_test_file");
        return NULL;
    }
    QDomElement testEl = doc.documentElement();
    return createTest(name, cp, env, testEl, err);
}

GTest* XMLTestFormat::createTest(const QString& name, GTest* cp, const GTestEnvironment* env, const QDomElement& el, QString& err) {
    QString tagName = el.tagName();
    XMLTestFactory* f = testFactories.value(tagName);
    if (f == NULL) {
        err = QString("XMLTestFactory not found '%1'").arg(tagName);
        return NULL;
    }
    QList<GTest*> subs;
    GTest* t = f->createTest(this, name, cp, env, subs, el);
    return t;
}

bool XMLTestFormat::registerTestFactory(XMLTestFactory* tf) {
    const QString& tagName = tf->getTagName();
    if (testFactories.contains(tagName)) {
        return false;
    }
    testFactories[tagName] = tf;
    return true;
}

bool XMLTestFormat::unregisterTestFactory(XMLTestFactory* tf) {
    const QString& tagName = tf->getTagName();
    if (!testFactories.contains(tagName)) {
        return false;
    }
    testFactories.remove(tagName);
    return true;
}

void XMLTestFormat::registerBuiltInFactories() {
    { // multitest
        QList<XMLTestFactory*> fs = XMLTestUtils::createTestFactories();
        foreach(XMLTestFactory* f, fs) { 
            bool res = registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }
}

}//namespace
