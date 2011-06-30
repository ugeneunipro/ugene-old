# include (workflow_dump.pri)

PLUGIN_ID=workflow_dump
PLUGIN_NAME=Workflow Dump
PLUGIN_VENDOR=Unipro

#Uncomment to enable workflow dump
#DEFINES+=WORKFLOW_DUMP

include( ../../ugene_plugin_common.pri )

INCLUDEPATH += ../../corelibs/U2Misc/_tmp