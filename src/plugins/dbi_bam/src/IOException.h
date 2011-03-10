#ifndef _U2_BAM_IO_EXCEPTION_H_
#define _U2_BAM_IO_EXCEPTION_H_

#include "Exception.h"

namespace U2 {
namespace BAM {

class IOException : public Exception
{
public:
    IOException(const QString &message);
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_IO_EXCEPTION_H_
