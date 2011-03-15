/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_BOWTIE_TESTS_H_
#define _U2_BOWTIE_TESTS_H_

#include <U2Test/XMLTestUtils.h>
#include <U2Core/GObject.h>
#include <QtXml/QDomElement>
#include <QFileInfo>

#include "BowtieTask.h"

namespace U2 {

class BowtieGObjectTask;
class DnaAssemblyMultiTask;
class MAlignmentObject;
class LoadDocumentTask;
class MAlignment;

class GTest_Bowtie : public GTest {
    Q_OBJECT
public:
	SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_Bowtie, "bowtie", TaskFlag_FailOnSubtaskCancel);
	~GTest_Bowtie();
    void prepare();
	void run();
    Task::ReportResult report();
    void cleanup();
    QString getTempDataDir();
	QList<Task*> onSubTaskFinished(Task* subTask);
	
private:    
	QList<DNASequence> dnaObjList_to_dnaList(QList<GObject*> dnaSeqs);
	void parseBowtieOutput(MAlignment& result, QString text);
	DnaAssemblyToRefTaskSettings config;
    QString readsFileName;
    GUrl readsFileUrl;
	QString indexName;
	QString patternFileName;
	QString negativeError;
	bool usePrebuildIndex;
	bool subTaskFailed;
	LoadDocumentTask* resultLoadTask;
	LoadDocumentTask* patternLoadTask;
	BowtieTask* bowtieTask;
	MAlignmentObject* ma1;
	MAlignmentObject* ma2;
	DocumentFormatId format;
	DocumentFormatId patternFormat;
};

class BowtieTests {
public:
	static QList<XMLTestFactory*> createTestFactories();
};
} //namespace U2

#endif //_U2_BOWTIE_TESTS_H_
