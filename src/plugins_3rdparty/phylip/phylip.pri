# include (umuscle.pri)

PLUGIN_ID=phylip
PLUGIN_NAME=Phylip
PLUGIN_VENDOR=Unipro
CONFIG += warn_off
include( ../../ugene_plugin_common.pri )

INCLUDEPATH += ../../corelibs/U2View/_tmp

win32-msvc2013 {
    DEFINES += NOMINMAX _XKEYCHECK_H
}
