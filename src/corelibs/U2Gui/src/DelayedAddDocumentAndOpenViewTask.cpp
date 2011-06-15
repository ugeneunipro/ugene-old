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

#include "DelayedAddDocumentAndOpenViewTask.h"

#include <U2Gui/OpenViewTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/MultiTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>

namespace U2 {

    void DelayedAddDocumentAndOpenViewTask::sl_onDocumentAvailable(Document *d) {
        assert(d != NULL);
        QList<Task*> taskList;

        // open a new project first if required
        Project* proj = AppContext::getProject();
        if (proj == NULL) {
            QList<GUrl> emptyList;
            taskList.prepend(  AppContext::getProjectLoader()->openProjectTask(emptyList, false) );
        }
        taskList.append(new LoadUnloadedDocumentAndOpenViewTask(d));
        taskList.append(new AddDocumentTask(d));
        addSubTask(new MultiTask(tr("Load and open the document"), taskList));
        AppContext::getTaskScheduler()->registerTopLevelTask(this);
    }
} // namespace