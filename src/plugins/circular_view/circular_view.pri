# include (circular_view.pri)

PLUGIN_ID=circular_view
PLUGIN_NAME=Plasmid viewer
PLUGIN_VENDOR=Unipro
PLUGIN_MODE=ui

include( ../../ugene_plugin_common.pri )

greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport
