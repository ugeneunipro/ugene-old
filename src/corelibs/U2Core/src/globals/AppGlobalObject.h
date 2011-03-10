#ifndef _U2_APP_GLOBAL_OBJECT_H_
#define _U2_APP_GLOBAL_OBJECT_H_

#include <U2Core/global.h>
#include <U2Core/Identifiable.h>

namespace U2 {

// Represents some Application global resource identified by ID.
// Only 1 resource of the given ID can exists per time

// Note: This entity class is separated into a separate class to 
// make it QObject and add more features to it in the future,  
// for example like deallocation features.

class U2CORE_EXPORT AppGlobalObject : public QObject, public Identifiable<QString> {
    Q_OBJECT

public:
    AppGlobalObject(const QString& id) : Identifiable<QString>(id) {}
};

} //namespace

#endif
