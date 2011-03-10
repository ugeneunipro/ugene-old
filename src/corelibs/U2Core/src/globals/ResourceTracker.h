#ifndef _U2_RESOURCE_TRACKER_H_
#define _U2_RESOURCE_TRACKER_H_

#include <U2Core/global.h>

#include <QtCore/QList>
#include <QtCore/QMap>

namespace U2 {


class Task;

class U2CORE_EXPORT ResourceTracker : public QObject {
    Q_OBJECT
public:
    ResourceTracker(){}
    ~ResourceTracker();

    void registerResourceUser(const QString& resourceName, Task* t);
    void unregisterResourceUser(const QString& resourceName, Task* t);

    QList<Task*> getResourceUsers(const QString& resourceName);

signals:
    void si_resourceUserRegistered(const QString& rName, Task* t);
    void si_resourceUserUnregistered(const QString& rName, Task* t);

private:
    typedef QList<Task*> TList;
    typedef QMap<QString, TList> RMap;

    RMap resMap;

};

} //namespace
#endif
