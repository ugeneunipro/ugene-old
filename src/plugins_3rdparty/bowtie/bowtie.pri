# include (bowtie.pri)
                        
DEFINES+= QT_DLL "BOWTIE_VERSION=\"\"0.12.7\"\"

PLUGIN_ID=bowtie
PLUGIN_NAME=Bowtie aligner
PLUGIN_VENDOR=Unipro
CONFIG += warn_off

include( ../../ugene_plugin_common.pri )
