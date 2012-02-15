# include (opencl_support.pri)


include( ../../ugene_globals.pri )

use_opencl() {

INCLUDEPATH += $$UGENE_OPENCL_INC_DIR

PLUGIN_ID=opencl_support
PLUGIN_NAME=OpenCL support
PLUGIN_VENDOR=Unipro
DEFINES += OPENCL_SUPPORT

include( ../../ugene_plugin_common.pri )


} #use_opencl
