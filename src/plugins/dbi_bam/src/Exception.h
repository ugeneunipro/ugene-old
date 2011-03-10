#ifndef _U2_BAM_EXCEPTION_H_
#define _U2_BAM_EXCEPTION_H_

#include <QtCore/QString>

namespace U2 {
namespace BAM {

class Exception
{
public:
    Exception(const QString &message);
    virtual ~Exception();
    const QString &getMessage()const;
private:
    QString message;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_EXCEPTION_H_
