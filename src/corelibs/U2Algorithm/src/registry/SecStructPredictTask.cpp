    #include "SecStructPredictTask.h"

namespace U2 {

SecStructPredictTask::SecStructPredictTask(const QByteArray& seq) 
: Task (tr("Secondary structure predict"), TaskFlag_None), sequence(seq)
{

}




} //namespace



