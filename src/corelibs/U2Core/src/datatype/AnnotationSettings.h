#ifndef _U2_ANNOTATION_SETTINGS_H_
#define _U2_ANNOTATION_SETTINGS_H_

#include <U2Core/global.h>

#include <QtCore/QHash>
#include <QtGui/QColor>

namespace U2 {

class Settings;
class Annotation;

class U2CORE_EXPORT AnnotationSettings {
public:
    AnnotationSettings();
    AnnotationSettings(const QString& name, bool amino, const QColor& color, bool visible);

    bool operator==(const AnnotationSettings* as) const {return equals(as);}
    bool equals(const AnnotationSettings* as) const;

    QString     name;
    QColor      color;
    bool        amino;
    bool        visible;
    QStringList nameQuals; //a value based on qualifiers value will be used instead of the annotation name
};

class U2CORE_EXPORT AnnotationSettingsRegistry : public QObject {
Q_OBJECT
public:
    AnnotationSettingsRegistry(const QList<AnnotationSettings*>& predefined);
    ~AnnotationSettingsRegistry();

    QStringList getAllSettings() const;
    AnnotationSettings* getAnnotationSettings(const QString& name);
    AnnotationSettings* getAnnotationSettings(Annotation* a);
    
    // persistent==true -> save settings to file, ==false -> this session only
    void changeSettings(const QList<AnnotationSettings*>& settings, bool saveAsPersistent);

signals:
    void si_annotationSettingsChanged(const QStringList& changedSettings);

private:
    void read();
    void save();

    QHash<QString, AnnotationSettings*> persistentMap;
    QHash<QString, AnnotationSettings*> transientMap;
};


}//namespace

#endif

