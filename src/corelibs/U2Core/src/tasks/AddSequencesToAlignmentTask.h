/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

namespace U2 {

class StateLock;
class LoadDocumentTask;
class U2SequenceObject;

class U2CORE_EXPORT AddSequenceObjectsToAlignmentTask : public Task {
    Q_OBJECT
public:
    AddSequenceObjectsToAlignmentTask(MAlignmentObject* obj, const QList<U2SequenceObject*>& seqList);
    virtual void prepare();

    ReportResult report();
protected:
    void processObjectsAndSetResultingAlphabet();

    QList<U2SequenceObject*>    seqList;
    QPointer<MAlignmentObject>  maObj;
private:
    StateLock*                  stateLock;
    const DNAAlphabet*          msaAlphabet;
    QStringList                 errorList;

    static const int maxErrorListSize;
    /** Returns the max length of the rows including trailing gaps */
    qint64 createRows(QList<U2MsaRow>& rows);
    void addRows(QList<U2MsaRow> &rows, qint64 len);
    void setupError();
    void releaseLock();
};

class U2CORE_EXPORT AddSequencesFromFilesToAlignmentTask : public AddSequenceObjectsToAlignmentTask {
    Q_OBJECT
public:
    AddSequencesFromFilesToAlignmentTask(MAlignmentObject* obj, const QStringList& urls);

    virtual void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    QStringList         urlList;
    LoadDocumentTask*   loadTask;
};

}// namespace

#endif //_U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_
