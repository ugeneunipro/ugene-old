#ifndef _U2_QSCORE_ADAPTER_H_
#define _U2_QSCORE_ADAPTER_H_

#include <U2Core/MAlignment.h>
#include <U2Core/Task.h>

namespace U2 {
	extern double QScore(const MAlignment& maTest, const MAlignment& maRef, TaskStateInfo& ti);
}

#endif //_U2_QSCORE_ADAPTER_H_
