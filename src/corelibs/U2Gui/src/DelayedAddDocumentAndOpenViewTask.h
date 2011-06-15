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

#ifndef _U2_DELAYED_ADD_AND_OPEN_VIEW_TASK_H_
#define _U2_DELAYED_ADD_AND_OPEN_VIEW_TASK_H_

#include <U2Core/Task.h>

namespace U2 {

class Document;

/**
    This task is a simple wrapper for an AddDocumentTask and a LoadUnloadedDocumentAndOpenViewTask
    which waits for a signal providing a document and then proceeds to create the two tasks as its
    subtasks and register itself with the task scheduler.
    It can only be deleted by the task scheduler so it's imperative that the correct signal is received
    at some point by an instance of this class and it's allowed to do its work and no memory leak occurs.
*/
class U2GUI_EXPORT DelayedAddDocumentAndOpenViewTask: public Task {
    Q_OBJECT
public:
    DelayedAddDocumentAndOpenViewTask() : Task(tr("Delayed load and open document task"), TaskFlags_NR_FOSCOE) {}
    ~DelayedAddDocumentAndOpenViewTask() {};

public slots:
    void sl_onDocumentAvailable(Document *d);
};


} // namespace


#endif
