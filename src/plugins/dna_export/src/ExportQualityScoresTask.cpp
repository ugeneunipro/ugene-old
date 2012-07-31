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

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <U2Core/Log.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/AppContext.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/L10n.h>
#include <U2Core/IOAdapterUtils.h>

#include "ExportQualityScoresTask.h"
#include <time.h>
#include <memory>

namespace U2 {


ExportPhredQualityScoresTask::ExportPhredQualityScoresTask( const U2SequenceObject* obj, const ExportQualityScoresConfig& cfg)
: Task("ExportPhredQuality", TaskFlag_None), seqObj(obj), config(cfg)
{

}

void ExportPhredQualityScoresTask::run() {
    
    DNAQuality seqQuality = seqObj->getQuality();
    QByteArray seqName = seqObj->getSequenceName().toAscii();

    if (seqQuality.isEmpty()) {
        stateInfo.setError("Quality score is not set!");
        return;
    }
    
    std::auto_ptr<IOAdapter> ioAdapter;

    IOAdapterId ioAdapterId = IOAdapterUtils::url2io(config.dstFilePath);
    IOAdapterFactory *ioAdapterFactory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(ioAdapterId);
    CHECK_EXT (ioAdapterFactory != NULL, stateInfo.setError(tr("No IO adapter found for URL: %1").arg(config.dstFilePath)), );
    ioAdapter.reset(ioAdapterFactory->createIOAdapter());

    if (!ioAdapter->open(config.dstFilePath, config.appendData ? IOAdapterMode_Append : IOAdapterMode_Write)) {
        stateInfo.setError(L10N::errorOpeningFileWrite(config.dstFilePath));
        return;
    }
    
    QByteArray data;
    data.append(">");
    data.append(seqName);
    data.append("\n");
    data.append(seqQuality.qualCodes);
    data.append("\n");

    if (0 == ioAdapter->writeBlock(data)) {
        stateInfo.setError(L10N::errorWritingFile(config.dstFilePath));
    }
   
    ioAdapter->close();

}






} // namespace U2
