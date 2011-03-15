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

#ifndef _U2_EDIT_SEQUENCE_TESTS_H_
#define _U2_EDIT_SEQUENCE_TESTS_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Core/GObject.h>

#include <U2Core/DNASequenceObject.h>

#include <U2Test/XMLTestUtils.h>

#include <QtXml/QDomElement>

#include <U2Core/RemovePartFromSequenceTask.h>
#include <U2Core/AddPartToSequenceTask.h>
#include <U2Core/ReplacePartOfSequenceTask.h>

namespace U2 {

class GTest_RemovePartFromSequenceTask : GTest {
Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_RemovePartFromSequenceTask, "Remove_Part_From_Sequence_Task", TaskFlags_NR_FOSCOE);

    ~GTest_RemovePartFromSequenceTask();

    void prepare();
    Task::ReportResult report();
private:
    U2AnnotationUtils::AnnotationStrategyForResize strat;
    QString             docName;
    QString             seqName;
    int                 startPos;
    int                 length;
    QString             annotationName;
    QString             expectedSequence;
    QVector<U2Region>      expectedRegions;
    DNASequenceObject*  dnaso;
};

class GTest_AddPartToSequenceTask : GTest {
Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_AddPartToSequenceTask, "Add_Part_To_Sequence_Task", TaskFlags_NR_FOSCOE);

    ~GTest_AddPartToSequenceTask();
    void prepare();
    Task::ReportResult report();
private:
    U2AnnotationUtils::AnnotationStrategyForResize strat;
    QString             docName;
    QString             seqName;
    int                 startPos;
    QString             insertedSequence;
    QString             annotationName;
    QString             expectedSequence;
    QVector<U2Region>      expectedRegions;
    DNASequenceObject*  dnaso;
};

class GTest_ReplacePartOfSequenceTask : GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_ReplacePartOfSequenceTask, "replace-part-of-sequence", TaskFlags_NR_FOSCOE);

    ~GTest_ReplacePartOfSequenceTask() {};
    void prepare();
    Task::ReportResult report();
private:
    U2AnnotationUtils::AnnotationStrategyForResize strat;
    QString             docName;
    QString             seqName;
    int                 startPos;
    int                 length;
    QString             insertedSequence;
    QString             annotationName;
    QString             expectedSequence;
    QVector<U2Region>      expectedRegions;
    DNASequenceObject*  dnaso;
};


class EditSequenceTests {
public:
    static QList< XMLTestFactory* > createTestFactories();

};
}//ns
#endif
