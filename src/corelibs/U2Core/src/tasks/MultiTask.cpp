#include "MultiTask.h"

namespace U2
{

MultiTask::MultiTask( const QString & name, const QList<Task *>& taskz ) : 
Task(name, TaskFlags_NR_FOSCOE), tasks(taskz)
{
    setMaxParallelSubtasks(1);
    if( taskz.empty() ) {
        assert( false ); 
        return;
    }

    foreach( Task * t, taskz ) {
        addSubTask(t);
    }
}

QList<Task*> MultiTask::getTasks() const {
    return tasks;
}

} //namespace
