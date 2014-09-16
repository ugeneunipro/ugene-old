# include (test_runner.pri)

QT += testlib
greaterThan(QT_MAJOR_VERSION, 4): QT += webkitwidgets

PLUGIN_ID=test_runner
PLUGIN_NAME=Test runner
PLUGIN_VENDOR=Unipro
PLUGIN_MODE=ui

include( ../../ugene_plugin_common.pri )
