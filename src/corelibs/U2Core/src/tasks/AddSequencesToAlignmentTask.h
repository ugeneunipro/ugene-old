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

#ifndef _U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_
#define _U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_

#include <U2Core/DNASequenceObject.h>
#include <U2Core/Task.h>
#include <U2Core/MAlignmentObject.h>

#include <QtCore/QPointer>


namespace U2 {

class LoadDocumentTask;
class StateLock;

class U2CORE_EXPORT AddSequencesToAlignmentTask : public Task {
    Q_OBJECT
public:
    AddSequencesToAlignmentTask(MAlignmentObject* obj, const QStringList& filesWithSequences);
    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);

    ReportResult report();

    QPointer<MAlignmentObject>  maObj;
    QStringList                 urls;
    StateLock*                  stateLock;

private:
    DNAAlphabet *msaAlphabet;
    QList<U2SequenceObject*>    seqList;
    QStringList                 errorList;

    static const int maxErrorListSize;

private:
    QList<U2MsaRow> createRows();
    void addRows(QList<U2MsaRow> &rows);
    void setupError();
};

}// namespace

#endif //_U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_
