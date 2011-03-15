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


#ifndef _U2_GET_DOCUMENT_FROM_INDEX_TASK_H_
#define _U2_GET_DOCUMENT_FROM_INDEX_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/UIndex.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

struct GZipIndexAccessPoint;

class U2CORE_EXPORT GetDocumentFromIndexTask : public Task {
    Q_OBJECT
public:
    GetDocumentFromIndexTask( const UIndex& index, int docNum );
    ~GetDocumentFromIndexTask();
    
    virtual void run();
    virtual ReportResult report();
    virtual void cleanup();
    
    Document* getDocument() const;
    Document* takeDocument();

private:
    bool fillAccessPointNums( GZipIndexAccessPoint& point, const QString& numStr );
    bool getGzipIndexAccessPoint( GZipIndexAccessPoint& ret, const UIndex::IOSection& ioSec, qint64 offset );
    IOAdapter* getOpenedIOAdapter(const UIndex::ItemSection& itemSec, const UIndex::IOSection& ioSec);
private:
    UIndex    index;
    int       docNum;
    Document* doc;
    
}; // GetDocumentFromIndexTask

} // U2

#endif // _U2_GET_DOCUMENT_FROM_INDEX_TASK_H_
