#ifndef _U2_DESIGNER_UTILS_H_
#define _U2_DESIGNER_UTILS_H_

#include <U2Core/global.h>
namespace U2 {

class U2DESIGNER_EXPORT DesignerUtils {
public:
    static QString getSchemaFileFilter(bool any, bool addOldExt = false);
}; // DesignerUtils

}//namespace

#endif

