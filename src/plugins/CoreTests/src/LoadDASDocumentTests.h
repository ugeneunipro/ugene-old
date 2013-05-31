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

#ifndef _U2_LOAD_DAS_DOCUMENT_TEST_H_
#define _U2_LOAD_DAS_DOCUMENT_TEST_H_

#include <U2Test/GTest.h>
#include <U2Test/XMLTestUtils.h>

namespace U2 {

    class LoadDASDocumentTask;

    class GTest_LoadDASDocumentTask : public GTest {
        Q_OBJECT
    public:
        SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_LoadDASDocumentTask, "load-das-document", TaskFlags_NR_FOSCOE);

        void prepare();
        ReportResult report();
    private:
        LoadDASDocumentTask *t;
        QString dbName;
        QString docId;
        QString expectedDoc;
    };

    class LoadDASDocumentTests {
    public:
        static QList<XMLTestFactory*> createTestFactories();
    };


} //namespace

#endif // _U2_LOAD_DAS_DOCUMENT_TEST_H_
