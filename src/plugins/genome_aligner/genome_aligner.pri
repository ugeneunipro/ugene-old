# include (genome_aligner.pri)

PLUGIN_ID=genome_aligner
PLUGIN_NAME=Genome aligner
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

use_opencl(){
    DEFINES += OPENCL_SUPPORT
}
