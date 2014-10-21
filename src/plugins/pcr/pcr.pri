# include (pcr.pri)

PLUGIN_ID=pcr
PLUGIN_NAME=In silico PCE
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport
