#ifndef _U2_ANNOTATOR_TESTS_H_
#define _U2_ANNOTATOR_TESTS_H_

#include <U2Test/XMLTestUtils.h>
#include <U2Core/GObject.h>
#include <QtXml/QDomElement>
#include <U2Core/U2Region.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2View/AnnotatedDNAView.h>
#include "CollocationsDialogController.h"

namespace U2 {

class GTest_AnnotatorSearch : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_AnnotatorSearch, "plugin_dna-annotator-search");

    void prepare();
    Task::ReportResult report();
private:
    QString seqName;
    QString docName;
    QString resultDocContextName;
    QSet<QString> groupsToSearch;
    int regionSize;
    CollocationsAlgorithm::SearchType st;
    CollocationSearchTask *searchTask;
    QVector<U2Region> expectedResults;
};

} //namespace
#endif
