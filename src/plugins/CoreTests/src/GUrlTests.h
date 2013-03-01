/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GURL_TESTS_H_
#define _U2_GURL_TESTS_H_

#include <U2Test/XMLTestUtils.h>
#include <QtXml/QDomElement>

namespace U2 {

class GTest_ConvertPath : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_ConvertPath, "convert-path");

    ReportResult report();

private:
    QString             originalUrl, result, expectedResult, platform;
    bool                isFileUrl, runThisTest;
};

class GTest_RemoveTmpDir : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_RemoveTmpDir, "remove-temp-dir");

    ReportResult report();

private:
    QString url;

private:
    void removeDir(const QString &url);
};

class GTest_RemoveTmpFile : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_RemoveTmpFile, "remove-temp-file");

    ReportResult report();

private:
    QString url;
};

class GTest_CreateTmpFile : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CreateTmpFile, "create-temp-file");

    ReportResult report();

private:
    QString url;
    QString data;
};

class GTest_CheckTmpFile : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CheckTmpFile, "check-temp-file");

    ReportResult report();

private:
    QString url;
    bool exists;
};

class GUrlTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

}//namespace
#endif
