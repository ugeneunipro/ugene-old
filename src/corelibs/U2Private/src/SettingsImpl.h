#ifndef _U2_SETTINGS_IMPL
#define _U2_SETTINGS_IMPL

#include "private.h"
#include <U2Core/Settings.h>

#include <QtCore/QSettings>
#include <QtCore/QMutex>

namespace U2 {

class U2PRIVATE_EXPORT SettingsImpl : public Settings {
public:
    SettingsImpl(QSettings::Scope scope);
    ~SettingsImpl();
    
    QStringList getAllKeys(const QString& path);

    virtual bool contains(const QString& key) const;
    virtual void remove(const QString& key);

    virtual QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    virtual void setValue(const QString& key, const QVariant& value);

    virtual QString toVersionKey(const QString& key) const;

    virtual void sync();
    
    virtual QString fileName() const;

private:
    mutable QMutex  threadSafityLock;
    QSettings* settings;
};
}//namespace
#endif
