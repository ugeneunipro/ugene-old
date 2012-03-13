#include (GUITestBase.pri)

PLUGIN_ID=GUITestBase
PLUGIN_NAME=GUI Test Base
PLUGIN_VENDOR=Unipro

unix {
    !macx {
    LIBS += -lXtst
    }
}

include( ../../ugene_plugin_common.pri )
