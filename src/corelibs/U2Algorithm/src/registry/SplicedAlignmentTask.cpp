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

#include "SplicedAlignmentTask.h"

namespace U2 {

SplicedAlignmentTask::SplicedAlignmentTask(const SplicedAlignmentTaskConfig& cfg) :
    Task(tr("SplicedAlignmentTask"), TaskFlag_None), config(cfg)
{

}

SplicedAlignmentTask::SplicedAlignmentTask(const QString &taskName, TaskFlags flags, const SplicedAlignmentTaskConfig &cfg) :
    Task(taskName, flags), config(cfg)
{
}

} //namespace
