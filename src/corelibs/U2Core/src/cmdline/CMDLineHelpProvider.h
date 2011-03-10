#ifndef _U2_CMD_LINE_HELP_PROVIDER_
#define _U2_CMD_LINE_HELP_PROVIDER_

#include "U2Core/global.h"

namespace U2 {

class U2CORE_EXPORT CMDLineHelpProvider {
public:
    CMDLineHelpProvider( const QString & n, const QString & c ) : name(n), content(c) {}
    
    QString getHelpSectionName() const { return name; }
    QString getHelpSectionContent() const { return content; }
    
private:
    QString name;
    QString content;
    
}; // CMDLineHelpProvider

}//namespace

#endif
