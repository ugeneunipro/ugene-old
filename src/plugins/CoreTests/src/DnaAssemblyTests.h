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

#ifndef _U2_DNA_ASSEMBLY_TESTS_H_
#define _U2_DNA_ASSEMBLY_TESTS_H_

#include <QtCore/QList>

#include <U2Test/XMLTestUtils.h>
#include <U2Core/GUrl.h>

namespace U2 {

class DnaAssemblyMultiTask;
class U2SequenceObject;
class LoadDocumentTask;

class GTest_DnaAssemblyToReferenceTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_DnaAssemblyToReferenceTask, "test-dna-assembly-to-ref");
    void prepare();
    Task::ReportResult report();
    void cleanup();
private:
    DnaAssemblyMultiTask* assemblyMultiTask;
    LoadDocumentTask* loadResultTask;
    QList<GUrl> shortReadUrls;
    QMap<QString,QString> customOptions;
    QString objName;
    QString algName;
    QString refSeqUrl;
    QString indexFileName;
    GUrl resultFileName;
    QString pairedReads;
};


class DnaAssemblyTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

} //namespace
#endif
