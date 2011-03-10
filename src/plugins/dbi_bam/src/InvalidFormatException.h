#ifndef _U2_BAM_INVALID_FORMAT_EXCEPTION_H_
#define _U2_BAM_INVALID_FORMAT_EXCEPTION_H_

#include "Exception.h"

namespace U2 {
namespace BAM {

class InvalidFormatException : public Exception
{
public:
    InvalidFormatException(const QString &message);
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_INVALID_FORMAT_EXCEPTION_H_
