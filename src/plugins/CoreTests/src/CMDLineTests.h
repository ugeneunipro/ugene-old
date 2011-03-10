#ifndef _U2_CMDLINE_TESTS_H_
#define _U2_CMDLINE_TESTS_H_

#include <QtCore/QProcess>
#include <U2Test/XMLTestUtils.h>

namespace U2 {

/***********************************
 * arguments for cmdline ugene are set in the following way:
 * tag="value" -> --tag=value
 * if value is existing file in _common_data we will resolve this
 * if you want to place file in _tmp you should write filename as "!tmp_out!filename"
 ***********************************/
class GTest_RunCMDLine : public GTest {
    Q_OBJECT
public:
    static const QString UGENECL_PATH;
    static const QString TMP_DATA_DIR_PREFIX;
    static const QString COMMON_DATA_DIR_PREFIX;
    
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY( GTest_RunCMDLine, "run-cmdline" );
    virtual void prepare();
    virtual ReportResult report();
    virtual void cleanup();
    
private:
    void setUgeneclPath();
    void setArgs( const QDomElement & owner );
    QString getVal( const QString & val );
    QString splitVal(const QString & val, int midSize, const QString & prefix, bool isTmp);
    
private:
    QString expectedMessage;
    QStringList args;
    QProcess *  proc;
    QString     ugeneclPath;
    QStringList tmpFiles;
    
}; // GTest_RunCMDLine

class CMDLineTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
}; // CMDLineTests

} // U2

#endif // _U2_CMDLINE_TESTS_H_
