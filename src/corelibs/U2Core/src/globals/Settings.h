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
    virtual void cleanSection(const QString& path) = 0;

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
