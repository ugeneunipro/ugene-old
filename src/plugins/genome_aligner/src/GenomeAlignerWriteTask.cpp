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

#include "GenomeAlignerWriteTask.h"

namespace U2 {

GenomeAlignerWriteTask::GenomeAlignerWriteTask(GenomeAlignerWriter *s)
: Task("WriteAlignedReadsSubTask", TaskFlag_None), seqWriter(s), end(false), writing(false)
{
}

void GenomeAlignerWriteTask::addResult(SearchQuery *qu) {
    listMutex.lock();
    WriteData data;

    foreach (SAType offset, qu->getResults()) {
        data.qu = qu;
        data.offset = offset;
        results.append(data);
    }

    if (!writing && results.size() > 1000) {
        writing = true;
        waiter.wakeAll();
    }
    listMutex.unlock();
}

void GenomeAlignerWriteTask::setFinished() {
    end = true;
    waiter.wakeAll();
}

void GenomeAlignerWriteTask::flush() {
    writeMutex.lock();
    foreach (WriteData data, results) {
        seqWriter->write(data.qu, data.offset);
    }
    results.clear();
    writeMutex.unlock();
}

void GenomeAlignerWriteTask::run() {
    do {
        waitMutex.lock();
        writing = false;
        waiter.wait(&waitMutex);
        if (end) {
            writing = false;
            listMutex.unlock();
            break;
        }

        QVector<WriteData> newResults;
        listMutex.lock();
        newResults += (results);
        results.clear();
        listMutex.unlock();
        waitMutex.unlock();

        writeMutex.lock();
        foreach (WriteData data, newResults) {
            seqWriter->write(data.qu, data.offset);
        }
        writeMutex.unlock();
    } while (!end);
}

} // U2
