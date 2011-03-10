#ifndef _U2_MULTI_TASK_H_
#define _U2_MULTI_TASK_H_

#include <U2Core/Task.h>

#include <QtCore/QList>
#include <QtCore/QString>

namespace U2
{

class U2CORE_EXPORT MultiTask : public Task {
    Q_OBJECT
public:
    MultiTask( const QString & name, const QList<Task *>& taskz );

    QList<Task*> getTasks() const;

private:
    QList<Task*> tasks;
    
};

} //namespace

#endif 
