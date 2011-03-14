# include (dbi_sqlite.pri)

PLUGIN_ID=dbi_sqlite
PLUGIN_NAME=SQLite based DBI implementation
PLUGIN_VENDOR=Unipro

LIBS += -lugene_db

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        LIBS -= -lugene_db
        LIBS += -lugene_dbd
    }
}


include( ../../ugene_plugin_common.pri )

INCLUDEPATH += ../../libs_3rdparty/sqlite3/src