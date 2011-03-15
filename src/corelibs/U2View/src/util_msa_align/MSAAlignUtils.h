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

#ifndef _U2_MSA_ALIGN_UTILS_H_
#define _U2_MSA_ALIGN_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/GUrl.h>
#include <U2Core/Task.h>

#include <U2Algorithm/MSAAlignTask.h>

namespace U2 {

class MSAAlignTaskSettings;
class Document;
class LoadDocumentTask;
class SaveDocumentTask;
class AddDocumentTask;
class MAlignmentObject;

class U2VIEW_EXPORT MSAAlignFileTask : public Task {
    Q_OBJECT
public:
    MSAAlignFileTask(const MSAAlignTaskSettings& settings, bool viewResult = false);
    virtual void prepare();
    virtual ReportResult report();
    QList<Task*> onSubTaskFinished(Task* subTask);
    const MAlignmentObject* getAlignResult();

private:
    MSAAlignTaskSettings settings;
    MSAAlignTask* alignTask;
    AddDocumentTask* addDocumentTask;
    LoadDocumentTask* loadDocumentTask;
    SaveDocumentTask* saveDocumentTask;
    MAlignmentObject* obj;
    Document* doc;
    bool openView;
}; 

} // U2

#endif // _U2_MSA_ALIGN_TASK_H_
