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

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/BaseDocumentFormats.h>

#include <U2Formats/BAMUtils.h>

#include "MergeBamTask.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
//MergeBamTask
MergeBamTask::MergeBamTask(const QStringList& urls, const QString &dir, const QString & outName)
: Task(tr("Merge BAM files with SAMTools merge"), TaskFlags_FOSCOE)
, outputName(outName)
, workingDir(dir)
, targetUrl("")
, bamUrls(urls)
{
    if (!workingDir.endsWith("/") && !workingDir.endsWith("\\")) {
        this->workingDir += "/";
    }
    if(outputName.isEmpty()){
        outputName == "merged.bam";
    }
}

QString MergeBamTask::getResult() const {
    return targetUrl;
}

void MergeBamTask::run(){
    if(bamUrls.isEmpty()){
        stateInfo.setError("No BAM files to merge");
        return;
    }

    targetUrl = workingDir + outputName;

    BAMUtils::mergeBam(bamUrls, targetUrl, stateInfo);
    CHECK_OP(stateInfo, );

    //TODO: bam merge assumes that a BAM files is sorted. Otherwise the sorting step can be added here

    BAMUtils::createBamIndex(targetUrl, stateInfo);

}
} // U2
