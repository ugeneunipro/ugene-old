# include (cuda_support.pri)


include( ../../ugene_globals.pri )

use_cuda() {

INCLUDEPATH += $$UGENE_CUDA_INC_DIR

PLUGIN_ID=cuda_support
PLUGIN_NAME=CUDA support
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )


} #use_cuda