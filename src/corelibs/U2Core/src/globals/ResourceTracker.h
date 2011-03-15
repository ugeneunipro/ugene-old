/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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
