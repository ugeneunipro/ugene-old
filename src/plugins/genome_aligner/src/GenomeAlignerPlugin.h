#ifndef _U2_GENOME_ALIGNER_PLUGIN_H_
#define _U2_GENOME_ALIGNER_PLUGIN_H_

#include <U2Core/PluginModel.h>

namespace U2 {

class GenomeAlignerPlugin : public Plugin  {
    Q_OBJECT
public:
    GenomeAlignerPlugin();
    ~GenomeAlignerPlugin();
};

} //namespace

#endif // _U2_GENOME_ALIGNER_PLUGIN_H_
