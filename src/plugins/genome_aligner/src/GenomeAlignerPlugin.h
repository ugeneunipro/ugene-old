#ifndef _U2_GENOME_ALIGNER_PLUGIN_H_
#define _U2_GENOME_ALIGNER_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Lang/Datatype.h>

namespace U2 {

class GenomeAlignerPlugin : public Plugin  {
    Q_OBJECT
public:
    GenomeAlignerPlugin();
    ~GenomeAlignerPlugin();

    static const QString GENOME_ALIGNER_INDEX_TYPE_ID;
    static DataTypePtr GENOME_ALIGNER_INDEX_TYPE();
};

} //namespace

#endif // _U2_GENOME_ALIGNER_PLUGIN_H_
