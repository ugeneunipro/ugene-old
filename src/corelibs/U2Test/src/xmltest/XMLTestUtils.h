/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_XML_TEST_UTILS_
#define _U2_XML_TEST_UTILS_

#include <U2Test/GTest.h>
#include "XMLTestFormat.h"

namespace U2 {


#define SIMPLE_XML_TEST_CONSTRUCT(ClassName, TFlags) \
    ClassName(XMLTestFormat* _tf, const QString& _name, GTest* _cp, \
        const GTestEnvironment* _env, const QList<GTest*>& _contexts, const QDomElement& _el) \
    : GTest(_name, _cp, _env, TFlags, _contexts){init(_tf, _el);} \


#define SIMPLE_XML_TEST_BODY(ClassName, TFlags) \
public:\
    SIMPLE_XML_TEST_CONSTRUCT(ClassName, TFlags) \
    void init(XMLTestFormat *tf, const QDomElement& el); \

    
#define SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(TestClass, TagName, TFlags) \
    SIMPLE_XML_TEST_BODY(TestClass, TFlags) \
    class TestClass##Factory : public XMLTestFactory { \
    public: \
        TestClass##Factory () : XMLTestFactory(TagName) {} \
        \
        virtual GTest* createTest(XMLTestFormat* tf, const QString& testName, GTest* cp, \
                    const GTestEnvironment* env, const QList<GTest*>& subtasks, const QDomElement& el) \
        { \
            return new TestClass(tf, testName, cp, env, subtasks, el); \
        }\
    };\
    \
    static XMLTestFactory* createFactory() {return new TestClass##Factory();}\


#define SIMPLE_XML_TEST_BODY_WITH_FACTORY(TestClass, TagName) \
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(TestClass, TagName, TaskFlags_NR_FOSCOE) \
    
    
    
class XMLTestUtils {
public:
    static QList<XMLTestFactory*> createTestFactories();
};


//////////////////////////////////////////////////////////////////////////
// utility tasks

class XMLMultiTest : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(XMLMultiTest, "multi-test");
    ReportResult report();
};

class GTest_Fail : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_Fail, "fail", TaskFlag_NoRun);
    ReportResult report();
private:
    QString msg;
};

class GTest_DeleteTmpFile : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_DeleteTmpFile, "delete", TaskFlag_NoRun);
    ReportResult report();
private:
    QString url;
};


} //namespace

#endif

