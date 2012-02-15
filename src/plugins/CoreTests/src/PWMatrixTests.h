/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PWMatrix_TESTS_H_
#define _U2_PWMatrix_TESTS_H_

#include <U2Test/XMLTestUtils.h>

#include <QtXml/QDomElement>

#include <U2Core/PFMatrix.h>
#include <U2Core/PWMatrix.h>

namespace U2 {

class Document;
class LoadDocumentTask;

//---------------------------------------------------------------------
class GTest_PFMtoPWMConvertTest : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_PFMtoPWMConvertTest, "check-pfm2pwm-convert");
private:
    QString             objContextName;
    PWMatrixType        type;
public:
    ReportResult        report();
};

class GTest_PFMCreateTest : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_PFMCreateTest, "check-pfm-create");
private:
    QString                 objContextName;
    QString                 objType;
    int                     size;
    int                     length;
    PFMatrixType            type;
    QVarLengthArray<int>    values[16];
public:
    ReportResult report();
};

class GTest_PWMCreateTest : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_PWMCreateTest, "check-pwm-create");

private:
    QString                 objContextName;
    QString                 objType;
    QString                 algo;
    int                     size;
    int                     length;
    PWMatrixType            type;
    QVarLengthArray<double> values[16];
public:
    ReportResult report();
};
//---------------------------------------------------------------------
class PWMatrixTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

}//namespace
#endif
