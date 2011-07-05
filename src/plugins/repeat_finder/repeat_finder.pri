# include (repeat_finder.pri)

PLUGIN_ID=repeat_finder
PLUGIN_NAME=Repeat finder
PLUGIN_VENDOR=Unipro
DEFINES+=GTEST_LINKED_AS_SHARED_LIBRARY=1
LIBS += -lgtest
include( ../../ugene_plugin_common.pri )

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS -= -lgtest
        LIBS += -lgtestd
    }
}

INCLUDEPATH += ../../libs_3rdparty/gtest/src