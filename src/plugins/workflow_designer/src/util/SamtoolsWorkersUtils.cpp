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

#include <U2Core/Log.h>

#include "SamtoolsWorkersUtils.h"

#define OUTPUT_SUBDIR "run"
namespace U2 {
namespace Workflow {

QString SamtoolsWorkerUtils::getFormatId(const FormatDetectionResult &r) {
    if (NULL != r.format) {
        return r.format->getFormatId();
    }
    if (NULL != r.importer) {
        return r.importer->getId();
    }
    return "";
}

QString SamtoolsWorkerUtils::createWorkingDir(const QString &fileUrl, int dirMode, const QString &customDir, const QString &workingDir){
    QString result;

    bool useInternal = false;

    if(dirMode == FILE_DIRECTORY){
        result = GUrl(fileUrl).dirPath() + "/";
    }else if (dirMode == CUSTOM){
        if (!customDir.isEmpty()){
            result = customDir;
            if (!result.endsWith("/")) {
                result += "/";
            }
        }else{
            algoLog.error("Result directory is empty, default workflow directory is used");
            useInternal = true;
        }
    }else{
        useInternal = true;
    }

    if (useInternal){
        result = workingDir;
        if (!result.endsWith("/")) {
            result += "/";
        }
        result += OUTPUT_SUBDIR;
    }

    QDir dir(result);
    if (!dir.exists(result)) {
        dir.mkdir(result);
    }
    return result;

}

QString SamtoolsWorkerUtils::detectFormat(const QString &url){
    FormatDetectionConfig cfg;
    cfg.bestMatchesOnly = false;
    cfg.useImporters = true;
    cfg.excludeHiddenFormats = false;

    const QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(url, cfg);
    if (formats.empty()) {
        return "";
    }

    return getFormatId(formats.first());
}


} // Workflow
} // U2
