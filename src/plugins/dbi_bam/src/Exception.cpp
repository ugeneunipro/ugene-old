#include "Exception.h"

namespace U2 {
namespace BAM {

Exception::Exception(const QString &message):
    message(message)
{
}

Exception::~Exception() {
}

const QString &Exception::getMessage()const {
    return message;
}

} // namespace BAM
} // namespace U2
