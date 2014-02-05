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

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <U2Formats/DNAQualityIOUtils.h>

#include "ExportQualityScoresTask.h"

namespace U2 {


ExportPhredQualityScoresTask::ExportPhredQualityScoresTask( const U2SequenceObject* obj, const ExportQualityScoresConfig& cfg)
: Task("ExportPhredQuality", TaskFlag_None), seqObj(obj), config(cfg)
{

}

void ExportPhredQualityScoresTask::run() {
    
    DNAQualityIOUtils::writeDNAQuality(seqObj, config.dstFilePath, config.appendData, false, stateInfo);

}






} // namespace U2
