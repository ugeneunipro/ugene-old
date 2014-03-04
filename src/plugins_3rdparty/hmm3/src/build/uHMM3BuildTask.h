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

#ifndef _UHMMER3_BUILD_TASK_H_
#define _UHMMER3_BUILD_TASK_H_

#include <QtCore/QMutex>

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/MAlignment.h>

#include <format/uHMMFormat.h>

#include <hmmer3/hmmer.h>

namespace U2 {

/* if we build multi-malignments file and error occurs while building -> we will return empty hmm list */
class UHMM3BuildTask : public Task {
    Q_OBJECT
public:
    UHMM3BuildTask( const UHMM3BuildSettings& settings, const MAlignment & msa );
    ~UHMM3BuildTask();
    virtual void run();
    P7_HMM * getHMM() const;
    P7_HMM * takeHMM();
    
private:
    bool checkMsa();
    void delHmm();
    
private:
    UHMM3BuildSettings  settings;
    MAlignment msa;
    P7_HMM * hmm;
    
}; // UHMM3BuildTask

class UHMM3BuildTaskSettings {
public:
    UHMM3BuildSettings  inner;
    QString             outFile;
    
    UHMM3BuildTaskSettings( const QString& outFile = QString() );
    
}; // UHMM3BuildTaskSettings

class UHMM3BuildToFileTask : public Task {
    Q_OBJECT
public:
    UHMM3BuildToFileTask( const UHMM3BuildTaskSettings& settings, const QList< MAlignment >& msas);
    UHMM3BuildToFileTask( const UHMM3BuildTaskSettings& settings, const MAlignment& ma );
    UHMM3BuildToFileTask( const UHMM3BuildTaskSettings& settings, const QString& in );
    
    virtual QList< Task* > onSubTaskFinished( Task* sub );
    
    QString generateReport() const;
    
private:
    void createBuildSubtasks();
    void addBuildSubTasks();
    
private:
    UHMM3BuildTaskSettings      settings;
    QString                     inFile;
    QList< MAlignment >         msas;
    QList<const P7_HMM* >            hmms;
    LoadDocumentTask*           loadTask;
    QList< UHMM3BuildTask* >    buildTasks;
    SaveDocumentTask*           saveHmmFileTask;
    Document*                   savingDocument;
    QMutex                      mtx;
    
}; // UHMM3BuildToFileTask

} // U2

#endif // _UHMMER3_BUILD_TASK_H_
