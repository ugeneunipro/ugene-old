# include (umuscle.pri)

PLUGIN_ID=umuscle
PLUGIN_NAME=Muscle3
PLUGIN_VENDOR=Unipro
CONFIG += warn_off

LIBS += -lqscore

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS -= -lqscore
        LIBS += -lqscored
    }
}


include( ../../ugene_plugin_common.pri )
