#include "CoreTests.h"
#include <U2Core/AppContext.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTestFrameworkComponents.h>

//built-in test impls
#include "EditSequenceTests.h"
#include "DocumentModelTests.h"
#include "DNASequenceObjectTests.h"
#include "AnnotationTableObjectTest.h"
#include "DNATranslationImplTests.h"
#include "BioStruct3DObjectTests.h"
#include "TaskTests.h"
#include "FilesIndexingTests.h"
#include "AsnParserTests.h"
#include "SequenceWalkerTests.h"
#include "PhyTreeObjectTests.h"
#include "PWMatrixTests.h"
#include "SecStructPredictTests.h"
#include "DnaAssemblyTests.h"

#include "EditAlignmentTests.h"
#include "SMatrixTests.h"
#include "BinaryFindOpenCLTests.h"

#include "LoadRemoteDocumentTests.h"
#include "CMDLineTests.h"

#include "GUrlTests.h"

namespace U2 {

extern "C" Q_DECL_EXPORT U2::Plugin* U2_PLUGIN_INIT_FUNC()
{
    return new CoreTests();
}

CoreTests::CoreTests() : Plugin("Core tests", "Core lib tests") {
    registerFactories();
}
CoreTests::~CoreTests() {
}

void CoreTests::registerFactories() {

    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    {
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = SMatrixTests::createTestFactories();

        foreach(XMLTestFactory* f, l->qlist) {
            bool res = xmlTestFormat->registerTestFactory(f);
            Q_UNUSED(res);
            assert(res);
        }
    }

    { //Document model tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = DocumentModelTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }
   { //DNASequenceObject Tests
       GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
       l->qlist = DNASequenceObjectTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }
    { //DNATranslationImplTests Tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = DNATranslationImplTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }
    { //Annotation Table Object Test
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = AnnotationTableObjectTest::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }
    { //Sequence Walker Test
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = SequenceWalkerTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }

    { //BioStruct3D Object Tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = BioStruct3DObjectTests::createTestFactories();
            foreach(XMLTestFactory* f, l->qlist) {
                bool res = xmlTestFormat->registerTestFactory(f);
                assert(res); Q_UNUSED(res);
            }
    }

    { //PhyTree Object Tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = PhyTreeObjectTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) {
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }

    { //Task Tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = TaskTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }
    { // Indexing files tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = FilesIndexingTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }
    
    { // Edit sequence tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = EditSequenceTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }
        
    { // AsnParser tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = AsnParserTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }

    { // SecStructPredict tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = SecStructPredictTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }

    { // DnaAssembly tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = DnaAssemblyTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }

    {
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = CreateSubalignimentTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }

    {
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = LoadRemoteDocumentTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }

     { // PWMatrix tests
         GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
         l->qlist = PWMatrixTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) { 
            bool res = xmlTestFormat->registerTestFactory(f);
            assert(res); Q_UNUSED(res);
        }
    }

     { // CMDLine tests
         GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
         l->qlist = CMDLineTests::createTestFactories();
         foreach(XMLTestFactory* f, l->qlist) { 
             bool res = xmlTestFormat->registerTestFactory(f);
             assert(res); Q_UNUSED(res);
         }
     }

     { // GUrl tests
         GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
         l->qlist = GUrlTests::createTestFactories();
         foreach(XMLTestFactory* f, l->qlist) { 
             bool res = xmlTestFormat->registerTestFactory(f);
             assert(res); Q_UNUSED(res);
         }
     }

    { // BinaryFindOpencl tests
        GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
        l->qlist = BinaryFindOpenCLTests::createTestFactories();
        foreach(XMLTestFactory* f, l->qlist) {
        bool res = xmlTestFormat->registerTestFactory(f);
        assert(res); Q_UNUSED(res);
    }
}
}

}//namespace
