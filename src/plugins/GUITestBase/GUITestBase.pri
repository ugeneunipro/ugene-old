#include (GUITestBase.pri)

PLUGIN_ID=GUITestBase
PLUGIN_NAME=GUI Test Base
PLUGIN_VENDOR=Unipro

INCLUDEPATH += ../../corelibs/U2View/_tmp/

unix {
    !macx {
    LIBS += -lXtst
    }
    macx {
    QMAKE_LFLAGS += -framework ApplicationServices
    }
}


include( ../../ugene_plugin_common.pri )
