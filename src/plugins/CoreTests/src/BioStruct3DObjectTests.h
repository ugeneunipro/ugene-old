#ifndef _U2_BIOSTRUCT3D_OBJECT_TESTS_H_
#define _U2_BIOSTRUCT3D_OBJECT_TESTS_H_

#include <U2Test/XMLTestUtils.h>
#include <QtXml/QDomElement>
#include <QtCore/QHash>

namespace U2 {

class Document;

class GTest_BioStruct3DNumberOfAtoms : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_BioStruct3DNumberOfAtoms, "check-biostruct3d-number-of-atoms");

    ReportResult report();

    QString objContextName;
    int numAtoms;

};

class GTest_BioStruct3DNumberOfChains : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_BioStruct3DNumberOfChains, "check-biostruct3d-number-of-chains");

    ReportResult report();

    QString objContextName;
    int     numChains;
};


class GTest_BioStruct3DAtomCoordinates : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_BioStruct3DAtomCoordinates, "check-biostruct3d-atom-coords");

    ReportResult report();

    QString objContextName;
    double  x,y,z;
    int     atomId;
    int     modelId;

};

class GTest_BioStruct3DAtomChainIndex : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_BioStruct3DAtomChainIndex, "check-biostruct3d-atom-chain-id");

    ReportResult report();

    QString objContextName;
    int     chainId;
    int     atomId;
    int     modelId;

};

class GTest_BioStruct3DAtomResidueName : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_BioStruct3DAtomResidueName, "check-biostruct3d-atom-residue-name");

    ReportResult report();

    QString objContextName;
    QString residueName;
    int     atomId;
    int     modelId;

};


class GTest_PDBFormatStressTest : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_PDBFormatStressTest, "try-load-all-pdb-documents");

    ReportResult report();
    QList<Task*> onSubTaskFinished(Task* subTask);
    QHash<Task*, QString>   fileNames;
};


class GTest_ASNFormatStressTest : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_ASNFormatStressTest, "try-load-all-mmdb-documents");

    ReportResult report();
    QList<Task*> onSubTaskFinished(Task* subTask);
    QHash<Task*, QString>   fileNames;
};


class BioStruct3DObjectTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};



}//namespace

#endif //_U2_BIOSTRUCT3D_OBJECT_TESTS_H_
