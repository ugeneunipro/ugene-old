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


#ifndef _U2_CREATE_FILE_INDEX_TASK_H_
#define _U2_CREATE_FILE_INDEX_TASK_H_

#include <QList>

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/UIndex.h>

namespace U2 {

class U2CORE_EXPORT CreateFileIndexTask : public Task {
    Q_OBJECT
public:
    static const QString KEY_NAME;
    static const QString KEY_SEQ_LEN;
    static const QString KEY_SEQ_COUNT;
    static const QString KEY_ACCESSION;
    static const QString KEY_DESCRIPTION;
    
public:
    CreateFileIndexTask( const QList< QString >& inputUrls, const QString& outputUrl, 
                         const QList< IOAdapterFactory* >& inputFactories, IOAdapterFactory* outputFactory );
    
    virtual void run();
    
    const QList<QString> & getInputUrls() const;
    const QString & getOutputUrl() const;
    
private:
    void readInputUrls();
    void readOneUrl( const QString& url, IOAdapterFactory* fa, int ind );
    void writeOutputUrl();
    void fillIOSec(UIndex::IOSection& ioSec, const QString& url, IOAdapterFactory* factory, int num );
    
private:
    QList< QString >  inputUrls;
    QString           outputUrl;
    
    QList< IOAdapterFactory* > inputFactories;
    IOAdapterFactory* outputFactory;
    
    UIndex            ind;
    
}; // CreateFileIndexTask

} // U2

#endif // _U2_CREATE_FILE_INDEX_TASK_H_
