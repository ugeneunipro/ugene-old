#ifndef _U2_DIPROPERTIES_TEST_H_
#define _U2_DIPROPERTIES_TEST_H_

#include "DIPropertiesSitecon.h"
#include <U2Test/XMLTestUtils.h>
#include <U2Core/GObject.h>
#include <QtXml/QDomElement>

namespace U2 {

class GTest_DiPropertySiteconCheckAttribs : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_DiPropertySiteconCheckAttribs, "sitecon-check_diproperty_attrib");

    Task::ReportResult report();
private:
    DinucleotitePropertyRegistry result;
    QString key;
    QString val;
    int expAve;
    int expSdev;
};

}//ns

#endif
