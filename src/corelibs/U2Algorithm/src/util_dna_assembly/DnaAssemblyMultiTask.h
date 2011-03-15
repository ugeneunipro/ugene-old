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

#ifndef _U2_DNA_ASSEMBLY_MULTI_TASK_
#define _U2_DNA_ASSEMBLY_MULTI_TASK_

#include <U2Algorithm/DnaAssemblyTask.h>
#include <U2Core/Task.h>
#include <U2Core/GUrl.h>

namespace U2 {

class Document;
class LoadDocumentTask;
class AddDocumentTask;
class MAlignmentObject;

class U2ALGORITHM_EXPORT DnaAssemblyMultiTask : public Task {
    Q_OBJECT
public:
    DnaAssemblyMultiTask(const DnaAssemblyToRefTaskSettings& settings, bool viewResult = false, bool justBuildIndex = false);
    virtual void prepare();
    virtual ReportResult report();
    virtual QString generateReport() const;
    QList<Task*> onSubTaskFinished(Task* subTask);
    const MAlignmentObject* getAssemblyResult();

signals:
    void documentAvailable(Document*);

private:
    DnaAssemblyToRefTaskSettings settings;
    DnaAssemblyToReferenceTask* assemblyToRefTask;
    AddDocumentTask* addDocumentTask;
    LoadDocumentTask* loadDocumentTask;
    Document* doc;
    QList<GUrl> shortReadUrls;
    bool openView;
    bool justBuildIndex;
}; 

} // namespace
#endif
