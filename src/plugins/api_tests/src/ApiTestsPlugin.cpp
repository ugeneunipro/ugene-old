#include "ApiTestsPlugin.h"
#include "ApiTest.h"

#include <U2Core/AppContext.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTestFrameworkComponents.h>

#include <core/dbi/ConvertToDbTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/DNAAlphabet.h>
namespace U2 {

extern "C" Q_DECL_EXPORT U2::Plugin *U2_PLUGIN_INIT_FUNC()
{
    return new ApiTestsPlugin();
}

ApiTestsPlugin::ApiTestsPlugin() :
Plugin("UGENE 2.0 API tests", "Tests for UGENE 2.0 public API") {
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);
    XMLTestFactory* f = GTest_APITest::createFactory();
    bool res = xmlTestFormat->registerTestFactory(f);
    assert(res); Q_UNUSED(res);
    //connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), SLOT(convert()));
}

void ApiTestsPlugin::convert() {
    /*QString file("E:/!sandbox/test-msa.aln");
    QString db("E:/!sandbox/alignment-dbi");
    AppContext::getTaskScheduler()->registerTopLevelTask(new ConvertToSQLiteTask(file, db));*/
    std::auto_ptr<U2Dbi> dbi;
    U2DbiFactory *factory = AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi");
    dbi.reset(factory->createDbi());

    QHash<QString, QString> props;
    props[U2_DBI_OPTION_CREATE] = U2_DBI_VALUE_ON;
    props[U2_DBI_OPTION_URL] = "E:/!sandbox/attribute-dbi.ugenedb";

    QVariantMap persistentData;
    U2OpStatusImpl os;

    dbi->init(props, persistentData, os);

    dbi->getObjectDbi()->createFolder("/", os);

    U2Sequence seq;
    seq.circular = true;
    seq.alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    U2SequenceDbi* seqDbi = dbi->getSequenceDbi();
    seqDbi->createSequenceObject(seq, "/", os);

    U2Assembly ass;
    U2AssemblyDbi* assDbi = dbi->getAssemblyDbi();
    U2AssemblyReadsImportInfo info;
    assDbi->createAssemblyObject(ass, "/", NULL, info, os);

    U2AttributeDbi* attrDbi = dbi->getAttributeDbi();

    U2IntegerAttribute int1;
    int1.objectId = seq.id;
    int1.name = "int1";
    int1.value = 5;
    int1.childId = ass.id;
    attrDbi->createIntegerAttribute(int1, os);

    U2IntegerAttribute int2;
    int2.objectId = seq.id;
    int2.name = "int2";
    int2.value = 3;
    int2.childId = ass.id;
    attrDbi->createIntegerAttribute(int2, os);

    U2RealAttribute real1;
    real1.objectId = seq.id;
    real1.name = "real1";
    real1.value = 2.7;
    real1.childId = ass.id;
    attrDbi->createRealAttribute(real1, os);

    U2StringAttribute str1;
    str1.objectId = seq.id;
    str1.name = "str1";
    str1.value = "some string";
    attrDbi->createStringAttribute(str1, os);

    U2StringAttribute str2;
    str2.objectId = seq.id;
    str2.name = "str2";
    str2.value = "other string";
    attrDbi->createStringAttribute(str2, os);

    U2ByteArrayAttribute arr1;
    arr1.objectId = seq.id;
    arr1.name = "arr1";
    arr1.value = "some array";
    attrDbi->createByteArrayAttribute(arr1, os);

    U2StringAttribute str;
    str.name = "str1";
    str.value = "some string";
    attrDbi->createStringAttribute(str, os);

    dbi->shutdown(os);
}

} // namespace U2
