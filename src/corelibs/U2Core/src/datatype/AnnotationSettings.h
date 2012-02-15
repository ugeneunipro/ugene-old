/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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

