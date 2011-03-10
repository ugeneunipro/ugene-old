# include (kalign.pri)

PLUGIN_ID=kalign
PLUGIN_NAME=KAlign
PLUGIN_VENDOR=Unipro


LIBS += -lqscore

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS -= -lqscore
        LIBS += -lqscored
    }
}


include( ../../ugene_plugin_common.pri )


