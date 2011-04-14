# include (dbi_file.pri)

PLUGIN_ID=dbi_file
PLUGIN_NAME=DBI implementation based on local files
PLUGIN_VENDOR=Unipro

LIBS += -lugenedb

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS -= -lugenedb
        LIBS += -lugenedbd
    }
}


include( ../../ugene_plugin_common.pri )
