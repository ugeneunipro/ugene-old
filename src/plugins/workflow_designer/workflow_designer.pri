# include (workflow_designer.pri)

PLUGIN_ID=workflow_designer
PLUGIN_NAME=Workflow Designer
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../Library/Frameworks/ -lQtScriptTools
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../Library/Frameworks/ -lQtScriptToolsd
else:mac: LIBS += -F$$PWD/../../../../../../../Library/Frameworks/ -framework QtScriptTools
else:unix: LIBS += -L$$PWD/../../../../../../../Library/Frameworks/ -lQtScriptTools

INCLUDEPATH += $$PWD/../../../../../../../Library/Frameworks
DEPENDPATH += $$PWD/../../../../../../../Library/Frameworks
