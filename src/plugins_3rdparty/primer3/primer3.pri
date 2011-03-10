# include (primer3.pri)

INCLUDEPATH += src/primer3_core 

PLUGIN_ID=primer3
PLUGIN_NAME=Primer3
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
}

