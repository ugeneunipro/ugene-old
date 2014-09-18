/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_DELETE_OBJECTS_H_
#define _U2_DELETE_OBJECTS_H_

#include <U2Core/Folder.h>
#include <U2Core/Task.h>
#include <U2Core/U2Type.h>

namespace U2 {

class GObject;

class U2CORE_EXPORT DeleteObjectsTask : public Task {
    Q_OBJECT
public:
    DeleteObjectsTask(const QList<GObject *> &objs);

    void run();

private:
    QList<U2EntityRef> objectRefs;
};

class U2CORE_EXPORT DeleteFoldersTask : public Task {
    Q_OBJECT
public:
    DeleteFoldersTask(const QList<Folder> &folders);

    void run();

private:
    QMultiMap<U2DbiRef, QString> dbi2Path;
};

}

#endif // _U2_DELETE_OBJECTS_H_
