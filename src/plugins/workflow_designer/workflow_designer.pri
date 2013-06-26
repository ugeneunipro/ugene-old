# include (workflow_designer.pri)

PLUGIN_ID=workflow_designer
PLUGIN_NAME=Workflow Designer
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

LIBS += -lQtScriptTools4

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS += -lQtScriptToolsd4
        LIBS -= -lQtScriptTools4
    }

}
