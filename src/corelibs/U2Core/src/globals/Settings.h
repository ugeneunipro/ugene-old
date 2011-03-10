#ifndef _U2_SETTINGS_H_
#define _U2_SETTINGS_H_

#include <U2Core/global.h>

#include <QtCore/QVariant>
#include <QtCore/QStringList>

namespace U2 {

class U2CORE_EXPORT Settings : public QObject {
    Q_OBJECT
public:
    virtual QStringList getAllKeys(const QString& path)  = 0;

    virtual bool contains(const QString& key) const = 0;
    virtual void remove(const QString& key) = 0;

    virtual QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const = 0;
    virtual void setValue(const QString& key, const QVariant& value) = 0;

    virtual QString toVersionKey(const QString& key) const = 0;
    
    virtual void sync()=0;
    virtual QString  fileName() const = 0;
};

} //namespace
#endif
