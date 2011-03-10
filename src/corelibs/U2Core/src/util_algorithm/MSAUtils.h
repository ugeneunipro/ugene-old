#ifndef _U2_MSA_UTILS_H_
#define _U2_MSA_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/DNASequence.h>

namespace U2 {

class GObject;
class MAlignment;
class MAlignmentRow;
class TaskStateInfo;

class U2CORE_EXPORT MSAUtils : public QObject {
    Q_OBJECT
public:

    static bool equalsIgnoreGaps(const MAlignmentRow& row, int startPos, const QByteArray& pattern);
    
    static MAlignment seq2ma(const QList<GObject*>& dnas, QString& err);

    static QList<DNASequence> ma2seq(const MAlignment& ma, bool trimGaps);

    //checks that alignment is not empty and all packed sequence parts has equal length
    static bool checkPackedModelSymmetry(MAlignment& ali, TaskStateInfo& ti);
};


}//namespace

#endif
