/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _GB2_UHMMER_PHMMER_TASK_H_
#define _GB2_UHMMER_PHMMER_TASK_H_

#include <QtCore/QList>
#include <QtCore/QMutex>

#include <U2Core/Task.h>
#include <U2Core/DNASequence.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/SequenceWalkerTask.h>

#include <search/uHMM3SearchTask.h>
#include "uhmm3phmmer.h"

namespace U2 {

class FeaturesTableObject;

/**************************************
 * General hmmer3 phmmer task.
 **************************************/
class UHMM3PhmmerTask : public Task {
    Q_OBJECT
public:
    static DNASequence getSequenceFromDocument( Document * doc, TaskStateInfo & ti );
    
public:
    UHMM3PhmmerTask( const DNASequence & query, const DNASequence & db, const UHMM3PhmmerSettings & settings );
    UHMM3PhmmerTask( const QString & queryFilename, const QString & dbFilename, const UHMM3PhmmerSettings & settings );
    UHMM3PhmmerTask( const QString & queryFilename, const DNASequence & db, const UHMM3PhmmerSettings & settings );
    
    void run();
    
    QList< Task* > onSubTaskFinished( Task* subTask );
    
    UHMM3SearchResult getResult() const;
    QList< SharedAnnotationData > getResultsAsAnnotations( const QString & name ) const;
    
private:
    void addMemResource();
    
private:
    DNASequence         query;
    DNASequence         db;
    UHMM3SearchResult   result;
    UHMM3PhmmerSettings settings;
    
    LoadDocumentTask *  loadQueryTask;
    LoadDocumentTask *  loadDbTask;
    QMutex              loadTasksMtx;
    
}; // UHMM3PhmmerTask

/******************************
 * Sequence walker phmmer
 ******************************/
class UHMM3SWPhmmerTask : public Task, SequenceWalkerCallback {
    Q_OBJECT
public:
    static const int DEFAULT_CHUNK_SIZE = 1000000; // 1mb
    
public:
    UHMM3SWPhmmerTask(const QString & queryFilename, const DNASequence & db, 
                      const UHMM3PhmmerSettings & settings, int chunk = DEFAULT_CHUNK_SIZE);
    
    QList<SharedAnnotationData> getResultsAsAnnotations(const QString & name) const;
    QList<UHMM3SWSearchTaskDomainResult> getResult()const;
    
    QList<Task*> onSubTaskFinished(Task* subTask);

    virtual void onRegion(SequenceWalkerSubtask * t, TaskStateInfo & ti);
    
    virtual QList<TaskResourceUsage> getResources(SequenceWalkerSubtask * t);
    
    ReportResult report();

private:
    SequenceWalkerTask* getSWSubtask();
    void checkAlphabets();
    void setTranslations();
    
private:
    QString             queryFilename;
    DNASequence         dbSeq;
    UHMM3PhmmerSettings settings;
    int                 searchChunkSize;
    LoadDocumentTask*   loadQueryTask;
    DNASequence         querySeq;
    SequenceWalkerTask* swTask;
    DNATranslation *    complTranslation;
    DNATranslation *    aminoTranslation;
    QMutex              writeResultsMtx;
    QList<UHMM3SWSearchTaskDomainResult> results;
    QList<UHMM3SWSearchTaskDomainResult> overlaps;
    
}; // UHMM3SWPhmmerTask

/*******************************************
* HMMER3 phmmer search to annotations task.
********************************************/

class UHMM3PhmmerToAnnotationsTask : public Task {
    Q_OBJECT
public:
    UHMM3PhmmerToAnnotationsTask( const QString & querySeqfile, const DNASequence & dbSeq, FeaturesTableObject *obj,
        const QString & group, const QString & name, const UHMM3PhmmerSettings & setings );
    
    QList< Task* > onSubTaskFinished( Task * subTask );
    
    QString generateReport() const;
    
private:
    void checkArgs();
    
private:
    QString                             queryfile;
    DNASequence                         dbSeq;
    QString                             annGroup;
    QString                             annName;
    UHMM3PhmmerSettings                 settings;
    QPointer<FeaturesTableObject>       annotationObj;
    UHMM3SWPhmmerTask *                 phmmerTask;
    CreateAnnotationsTask *             createAnnotationsTask;
    
}; // UHMM3PhmmerToAnnotationsTask

} // U2

#endif // _GB2_UHMMER_PHMMER_TASK_H_
