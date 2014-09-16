# include (workflow_designer.pri)

PLUGIN_ID=workflow_designer
PLUGIN_NAME=Workflow Designer
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

QT += scripttools
greaterThan(QT_MAJOR_VERSION, 4): QT += webkitwidgets
