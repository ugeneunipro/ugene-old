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

#ifndef _U2_EXPORT_CONSENSUS_TASK_H_
#define _U2_EXPORT_CONSENSUS_TASK_H_

#include "AssemblyConsensusTask.h"

#include <QtCore/QQueue>

#include <U2Core/DocumentProviderTask.h>
#include <U2Core/U2SequenceUtils.h>

namespace U2 {

struct ExportConsensusTaskSettings : public AssemblyConsensusTaskSettings {
    DocumentFormatId formatId;
    QString fileName;
    QString seqObjName;
    bool addToProject;
    bool keepGaps;
    bool circular;
};

class ExportConsensusTask : public DocumentProviderTask, ConsensusSettingsQueue {
public:
    ExportConsensusTask(const ExportConsensusTaskSettings &settings_);

    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);

    // implement ConsensusSettingsQueue interface
    virtual int count() { return consensusRegions.count(); }
    virtual bool hasNext() { return ! consensusRegions.isEmpty(); }
    virtual AssemblyConsensusTaskSettings getNextSettings();
    virtual void reportResult(const ConsensusInfo &result);

private:
    ExportConsensusTaskSettings settings;
    AssemblyConsensusWorker * consensusTask;
    U2SequenceImporter seqImporter;

    // A region to analyze at a time
    static const qint64 REGION_TO_ANALAYZE = 1000000;

    // implement ConsensusSettingsQueue:
    QQueue<U2Region> consensusRegions;
};

} // namespace

#endif
