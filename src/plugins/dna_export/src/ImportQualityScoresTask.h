/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_IMPORT_QUALITY_SCORES_TASK_H_
#define _U2_IMPORT_QUALITY_SOCRES_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DNAQuality.h>

#include <QtCore/QPointer>

namespace U2 {

class U2SequenceObject;

class ImportQualityScoresConfig {
public:
    ImportQualityScoresConfig() : createNewDocument(false) {}
    QString             fileName;
    DNAQualityType      type; 
    bool                createNewDocument;
    QString             dstFileName;
};

 
class ReadQualityScoresTask : public Task {
    Q_OBJECT
public:
    ReadQualityScoresTask(const QString& fileName, DNAQualityType t);

    void run();
    
    QMap<QString,DNAQuality> getResult() const { return result; }
    
private:
    void recordQuality( int headerCounter );
    QString                     fileName;
    DNAQualityType              type;
    QStringList                 headers;
    QList<int>                  values;
    QMap<QString, DNAQuality>   result;
};


class ImportPhredQualityScoresTask : public Task {
    Q_OBJECT
public:
    ImportPhredQualityScoresTask(const QList<U2SequenceObject*>& sequences, ImportQualityScoresConfig& config);

    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);

private:
    ReadQualityScoresTask* readQualitiesTask;
    ImportQualityScoresConfig config;
    QList<U2SequenceObject*> seqList;
};


} // namespace U2

#endif 
