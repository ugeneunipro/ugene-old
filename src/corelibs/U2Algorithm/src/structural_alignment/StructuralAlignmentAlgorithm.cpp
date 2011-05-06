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

#include "StructuralAlignmentAlgorithm.h"

namespace U2 {

/* class U2ALGORITHM_EXPORT StructuralAlignmentTask : public Task */
StructuralAlignmentTask::StructuralAlignmentTask(StructuralAlignmentAlgorithm *_algorithm, const StructuralAlignmentTaskSettings &_settings)
        : Task("StructuralAlignmentTask", TaskFlag_None), algorithm(_algorithm), settings(_settings)
{
}

void StructuralAlignmentTask::run() {
    result = algorithm->align(settings);
}

}   // namespace U2
