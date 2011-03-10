#include "InvalidFormatException.h"

namespace U2 {
namespace BAM {

InvalidFormatException::InvalidFormatException(const QString &message):
    Exception(message)
{
}

} // namespace BAM
} // namespace U2
