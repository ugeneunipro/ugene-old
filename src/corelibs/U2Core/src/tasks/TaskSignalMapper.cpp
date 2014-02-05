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

#include "TaskSignalMapper.h"

namespace U2 {

void TaskSignalMapper::sl_taskStateChanged() {
    QObject* o = sender();
    if (o) {
        Task* t = qobject_cast<Task*>(o);
        if (t) {
            switch (t->getState()) {
            case Task::State_Prepared:
                emit si_taskPrepared(t);
                break;
            case Task::State_Running:
                emit si_taskRunning(t);
                break;
            case Task::State_Finished:
                emit si_taskFinished(t);
                if (t->hasError() || t->hasSubtasksWithErrors()) {
                    emit si_taskFailed(t);
                } else {
                    emit si_taskSucceeded(t);
                }
                break;
            case Task::State_New:
                break;
            }
        }
    }
}

} //namespace
