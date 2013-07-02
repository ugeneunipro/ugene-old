# include (U2Script.pri)

MODULE_ID=U2Script

include( ../../ugene_lib_common.pri )

UGENE_RELATIVE_DESTDIR = ''

DEFINES +=          QT_FATAL_ASSERT BUILDING_U2SCRIPT_DLL

use_opencl(){
    DEFINES +=      OPENCL_SUPPORT
}

QT -= gui

INCLUDEPATH +=      ../../include \
                    ../U2Private/src

#count( UGENE_NODE_DIR, 1 ) {
#    QMAKE_EXTENSION_SHLIB = node
#
#    INCLUDEPATH +=  $${UGENE_NODE_DIR}/src \
#                    $${UGENE_NODE_DIR}/deps/v8/include \
#                    $${UGENE_NODE_DIR}/deps/uv/include
#}

!debug_and_release|build_pass {

    CONFIG( debug, debug|release ) {
        DESTDIR =   ../../_debug

        LIBS +=     -L../../_debug -lU2Cored -lU2Algorithmd -lU2Formatsd -lU2Remoted -lU2Langd -lU2Privated -lugenedbd -lU2Guid -lU2Testd
                    
#        count( UGENE_NODE_DIR, 1 ) {
#            LIBS += -l$${UGENE_NODE_DIR}/Debug/node
#        }
    }

    CONFIG( release, debug|release ) {
        DESTDIR =   ../../_release

        LIBS +=     -L../../_release -lU2Core -lU2Algorithm -lU2Formats -lU2Remote -lU2Lang -lU2Private -lugenedb -lU2Gui -lU2Test

#        count( UGENE_NODE_DIR, 1 ) {
#            LIBS += -l$${UGENE_NODE_DIR}/Release/node
#        }
    }
}

unix {
    target.path =   $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS +=     target
}