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


#ifndef _U2_EXPORT_TO_NEW_FILE_FROM_INDEX_TASK_H_
#define _U2_EXPORT_TO_NEW_FILE_FROM_INDEX_TASK_H_

#include <U2Core/DocumentModel.h>

#include <U2Core/Task.h>
#include <U2Core/UIndex.h>

namespace U2 {

class IOAdapter;
class GetDocumentFromIndexTask;
class SaveDocumentStreamingTask;

/*
 * Exported documents may be written to filename not in docNums list order
 */
class U2CORE_EXPORT ExportToNewFileFromIndexTask : public Task {
    Q_OBJECT
public:
    ExportToNewFileFromIndexTask( const UIndex& index, const QList< int >& docNums, const QString& filename );
    ~ExportToNewFileFromIndexTask();
    
    virtual void prepare();
    virtual ReportResult report();
    
private:
    QList< Task* > getDocTaskFinished( GetDocumentFromIndexTask* t );
    QList< Task* > saveDocTaskFinished( SaveDocumentStreamingTask* t );
    
protected:
    virtual QList< Task* > onSubTaskFinished( Task* subTask );
    IOAdapter* getOpenedIOAdapter(const QString& url);
    
private:
    UIndex             index;
    QList< int >       docNums;
    QString            exportFilename;
    QList< Document* > docsToSave;
    
    IOAdapter*                 io;
    SaveDocumentStreamingTask* saveCurDocTask;
    
}; // ExportToNewFileFromIndexTask

} // U2

#endif // _U2_EXPORT_TO_NEW_FILE_FROM_INDEX_TASK_H_
