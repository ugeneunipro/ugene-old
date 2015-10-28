/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef GTUTILSTASK_H
#define GTUTILSTASK_H

#include "GTGlobals.h"

class Task;

namespace U2 {

class GTUtilsTask
{
public:
    static QList<Task*> getTopLevelTasks(U2OpStatus &os);
    static Task* getTaskByName(U2OpStatus &os, QString taskName, GTGlobals::FindOptions options = GTGlobals::FindOptions(true));
    static Task* getSubTaskByName(U2OpStatus &os, QString taskName, GTGlobals::FindOptions options = GTGlobals::FindOptions(true));
    static Task* getSubTaskByName(U2OpStatus &os, Task* parent, QString taskName, GTGlobals::FindOptions options = GTGlobals::FindOptions(true));
    static void checkTask(U2OpStatus &os, QString taskName);
    static void checkNoTask(U2OpStatus &os, QString taskName);
    static void cancelTask(U2OpStatus &os, QString taskName);
    static void cancelSubTask(U2OpStatus &os, QString taskName);
    static void waitTaskStart(U2OpStatus &os, QString taskName, int timeOut = 180000);

};

}

#endif // GTUTILSTASK_H
