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

#include <memory>

#include "AssemblyConsensusTask.h"

#include <U2Core/Log.h>
#include <U2Core/Timer.h>
#include <U2Core/U2OpStatusUtils.h>

namespace U2 {

AssemblyConsensusTask::AssemblyConsensusTask(const AssemblyConsensusTaskSettings &settings_)
    : BackgroundTask<QByteArray>("Calculate assembly consensus", TaskFlag_None), settings(settings_)
{
    tpm = Progress_Manual;
}

void AssemblyConsensusTask::run() {
    GTIMER(c2, t2, "AssemblyConsensusTask::run");
    quint64 t0 = GTimer::currentTimeMicros();

    U2OpStatus2Log os;
    std::auto_ptr< U2DbiIterator<U2AssemblyRead> > reads(settings.model->getReads(settings.region, os));

    result = settings.consensusAlgorithm->getConsensusRegion(settings.region, reads.get(), stateInfo);
            
    perfLog.trace(QString("Assembly: '%1' consensus calculation time: %2 seconds")
                  .arg(settings.consensusAlgorithm->getName())
                  .arg((GTimer::currentTimeMicros() - t0) / float(1000*1000)));

}

} //namespace
