#ifndef _U2_BAM_CONVERT_TO_SQLITE_TASK_H_
#define _U2_BAM_CONVERT_TO_SQLITE_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/GUrl.h>

namespace U2 {
namespace BAM {

class ConvertToSQLiteTask : public Task
{
public:
    ConvertToSQLiteTask(const GUrl &sourceUrl, const QList<GUrl> referenceUrls, const GUrl &destinationUrl);

    virtual void run();
private:
    GUrl sourceUrl;
    GUrl destinationUrl;
    QList<GUrl> referenceUrls;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_CONVERT_TO_SQLITE_TASK_H_
