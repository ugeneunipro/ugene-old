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

#include "WriteAlignedReadsSubTask.h"

namespace U2 {

WriteAlignedReadsSubTask::WriteAlignedReadsSubTask(QMutex &_listM, GenomeAlignerWriter *_seqWriter, QVector<SearchQuery*> &_queries, quint64 &r)
: Task("WriteAlignedReadsSubTask", TaskFlag_None), seqWriter(_seqWriter), queries(_queries), readsAligned(r), listM(_listM)
{

}

void WriteAlignedReadsSubTask::setReadWritten(SearchQuery *read, SearchQuery *revCompl) {
    if (!read->isWroteResult()) {
        readsAligned++;
        read->writeResult();
        if (NULL != revCompl) {
            revCompl->writeResult();
        }
    }
}

void WriteAlignedReadsSubTask::run() {
    // ReadShortReadsSubTask can add new data what can lead to realloc. Noone can touch these vectors without sync
    QMutexLocker lock(&listM);

    stateInfo.setProgress(0);
    SearchQuery *read = NULL;
    SearchQuery *revCompl = NULL;
    SearchQuery **q = queries.data();
    int size = queries.size();

    try {
        for (int i=0; i<size; i++) {
            read = q[i];
            revCompl = read->getRevCompl();

            if (i<size-1 && revCompl == q[i+1]) {
                continue;
            }

            if (NULL == revCompl && read->haveResult()) {
                seqWriter->write(read, read->firstResult());
                readsAligned++;
            } else if (NULL != revCompl) {
                int c = read->firstMCount();
                int cRev = revCompl->firstMCount();

                if (c <= cRev && c < INT_MAX) {
                    seqWriter->write(read, read->firstResult());
                    readsAligned++;
                } else if (cRev < INT_MAX) {
                    seqWriter->write(revCompl, revCompl->firstResult());
                    readsAligned++;
                }
            }
        }
    } catch (QString exeptionMessage) {
        setError(exeptionMessage);
        return;
    }
}

} // U2
