#ifndef _U2_KALIGN_ADAPTER_H_
#define _U2_KALIGN_ADAPTER_H_

#include <QtCore/QObject>

namespace U2 {

class MAlignment;
class TaskStateInfo;

class KalignAdapter : public QObject {
	Q_OBJECT
public:
	static void align(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti);

private:
	static void alignUnsafe(const MAlignment& ma, MAlignment& res, TaskStateInfo& ti);
};

}//namespace

#endif

