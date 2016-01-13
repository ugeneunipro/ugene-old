/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_
#define _U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_

#include <QPointer>

#include <U2Core/Task.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2Type.h>

namespace U2 {

class StateLock;
class LoadDocumentTask;
class DNASequence;
class U2MsaDbi;
class U2UseCommonUserModStep;

class U2CORE_EXPORT AddSequenceObjectsToAlignmentTask : public Task {
    Q_OBJECT
public:
    AddSequenceObjectsToAlignmentTask(MAlignmentObject* obj, const QList<DNASequence>& seqList);

    virtual void prepare();
    virtual void run();
    ReportResult report();
protected:
    void processObjectsAndSetResultingAlphabet();

    QList<DNASequence>    seqList;
    QPointer<MAlignmentObject>  maObj;
protected:
    void releaseLock();
private:
    StateLock*                  stateLock;
    const DNAAlphabet*          msaAlphabet;
    QStringList                 errorList;    
    U2MsaDbi*                   dbi;
    U2EntityRef                 entityRef;
    U2UseCommonUserModStep*     modStep;
    MAlignmentModInfo           mi;

    static const int maxErrorListSize;
    /** Returns the max length of the rows including trailing gaps */
    qint64 createRows(QList<U2MsaRow> &rows);
    void addRows(QList<U2MsaRow> &rows, qint64 maxLength);
    void updateAlphabet();
    void setupError();
};

class U2CORE_EXPORT AddSequencesFromFilesToAlignmentTask : public AddSequenceObjectsToAlignmentTask {
    Q_OBJECT
public:
    AddSequencesFromFilesToAlignmentTask(MAlignmentObject* obj, const QStringList& urls);

    virtual void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);
private slots:
    void sl_onCancel();
private:
    QStringList         urlList;
    LoadDocumentTask*   loadTask;
};

class U2CORE_EXPORT AddSequencesFromDocumentsToAlignmentTask : public AddSequenceObjectsToAlignmentTask {
    Q_OBJECT
public:
    AddSequencesFromDocumentsToAlignmentTask(MAlignmentObject* obj, const QList<Document*>& docs);

    virtual void prepare();
private:
    QList<Document*> docs;
};

}// namespace

#endif //_U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_
