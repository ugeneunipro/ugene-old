#ifndef _U2_FORMATDB_SUPPORT_H
#define _U2_FORMATDB_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include "utils/ExternalToolSupportAction.h"

#define FORMATDB_TOOL_NAME "FormatDB"
#define MAKEBLASTDB_TOOL_NAME "MakeBLASTDB"
namespace U2 {

class FormatDBSupport : public ExternalTool {
    Q_OBJECT
public:
    FormatDBSupport(const QString& name, const QString& path = "");

public slots:
    void sl_runWithExtFileSpecify();
};

}//namespace
#endif // _U2_FORMATDB_SUPPORT_H
