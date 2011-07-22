#ifndef _U2_BOWTIE_H_
#define _U2_BOWTIE_H_

#include "U2Core/ExternalToolRegistry.h"

namespace U2 {

#define BOWTIE_TOOL_NAME "Bowtie"
#define BOWTIE_BUILD_TOOL_NAME "Bowtie-build"

class BowtieSupport : public ExternalTool {
    Q_OBJECT
public:
    BowtieSupport(const QString &name, const QString &path = "");
};

} // namespace U2

#endif // _U2_BOWTIE_H_
