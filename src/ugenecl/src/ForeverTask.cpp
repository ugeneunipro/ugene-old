
#include <QtCore/QEventLoop>

#include <U2Core/Log.h>

#include "ForeverTask.h"

#define FOREVER_TASK_LOG_CAT "Forever task"

namespace U2 {

Logger log( FOREVER_TASK_LOG_CAT );

ForeverTask::ForeverTask() : Task( "Forever task", TaskFlag_None ) {
    log.trace( tr( "Forever task created" ) );
}

ForeverTask::~ForeverTask() {
}

void ForeverTask::run() {
    QEventLoop loop;
    loop.exec();
}

} // U2
