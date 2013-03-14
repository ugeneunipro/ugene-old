MODULE_ID=$${PLUGIN_ID}
include (ugene_lib_common.pri)

# This file is common for all UGENE plugins

UGENE_RELATIVE_DESTDIR = 'plugins'
QT += network xml webkit svg
LIBS += -L../../_release -lU2Core -lU2Algorithm -lU2Formats -lU2Gui -lU2View -lU2Test -lU2Remote -lU2Lang -lU2Designer

!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        PLUGIN_ID=$$join(PLUGIN_ID, "", "", "d")
        DESTDIR=../../_debug/plugins
        LIBS -= -L../../_release -lU2Core -lU2Algorithm -lU2Formats -lU2Gui -lU2View -lU2Test -lU2Remote -lU2Lang -lU2Designer
        LIBS += -L../../_debug -lU2Cored -lU2Algorithmd -lU2Formatsd -lU2Guid -lU2Viewd -lU2Testd -lU2Remoted -lU2Langd -lU2Designerd
    }
    CONFIG(release, debug|release) {
        DESTDIR=../../_release/plugins
    }
    
    # The directory 'plugins' must exist for plugin desc file   
    unix {
    !exists( $$CONFDIR/plugins/*.plugin ) {
            system( mkdir $$CONFDIR/plugins )
        }
    }
    
    win32 {
    !exists( $$CONFDIR/plugins/*.plugin ) {
            system( mkdir $$CONFDIR\\plugins )
        }
    }

    include (./ugene_plugin_descriptor.pri)
}

win32 {
    QMAKE_MSVC_PROJECT_NAME=plugin_$${PLUGIN_ID}
    LIBS += psapi.lib
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
