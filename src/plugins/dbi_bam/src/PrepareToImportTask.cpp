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


#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/BAMUtils.h>


#include "PrepareToImportTask.h"

namespace U2 {
namespace BAM {

PrepareToImportTask::PrepareToImportTask( const GUrl& url, bool sam, const QString& refUrl ) : Task("Prepare assembly file to import", TaskFlag_None), 
                                                                        sourceURL( url ), refUrl(refUrl), samFormat(sam), newURL(false)
{ 
    tpm = Progress_Manual; 
}

void PrepareToImportTask::run() {
    GUrl currentURL;
    QString destinationURL ;
    QString dirPath = QString();
    U2OpStatusImpl status;

    currentURL = sourceURL;
    if( samFormat == true ) {
        checkReferenceFile();
        CHECK_OP(stateInfo, );

        dirPath = sourceURL.dirPath();
        if( !dirPath.endsWith("/") ) {
            dirPath.append("/");
        }
        destinationURL = dirPath + currentURL.fileName() + ".bam";
        stateInfo.setDescription( "Converting SAM to BAM" );
        BAMUtils::ConvertOption options(true, refUrl);
        BAMUtils::convertToSamOrBam( currentURL, destinationURL, options, stateInfo );
        CHECK_OP(stateInfo, );
        if( !newURL ) {
            newURL = true;
        }
        currentURL = destinationURL;
    }
    stateInfo.setProgress( 33 );
    bool sortedBam = BAMUtils::isSortedBam( currentURL, stateInfo );
    CHECK_OP(stateInfo, );
    if( !sortedBam ) {
        const QString sortedFileName = dirPath + currentURL.fileName() + "_sorted";
        stateInfo.setDescription( "Sorting BAM" );
        currentURL = BAMUtils::sortBam( currentURL, sortedFileName, stateInfo );
        CHECK_OP(stateInfo, );
        if( !newURL ) {
            newURL = true;
        }
    } 
    stateInfo.setProgress( 66 );
    if( !BAMUtils::hasValidBamIndex( currentURL ) ) {
        stateInfo.setDescription( "Creating BAM index" );
        BAMUtils::createBamIndex( currentURL, stateInfo );
        CHECK_OP(stateInfo, );
    }
    stateInfo.setProgress( 100 );
    sourceURL = currentURL;
}

namespace {
static bool isUnknownFormat(const QList<FormatDetectionResult> &formats) {
    if (formats.isEmpty()) {
        return true;
    }
    FormatDetectionResult f = formats.first();
    if (NULL == f.format && NULL == f.importer) {
        return true;
    }
    return false;
}

static QString detectedFormatId(const FormatDetectionResult &f) {
    if (NULL == f.format && f.importer == NULL) {
        return "";
    } else if (NULL == f.format) {
        return f.importer->getImporterName();
    }
    return f.format->getFormatId();
}
}

void PrepareToImportTask::checkReferenceFile() {
    CHECK(!refUrl.isEmpty(), );

    FormatDetectionConfig cfg;
    cfg.useImporters = true;
    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(refUrl, cfg);
    if (isUnknownFormat(formats)) {
        setError(tr("Unknown reference sequence format. Only FASTA is supported"));
        return;
    }
    QString formatId = detectedFormatId(formats.first());
    if (BaseDocumentFormats::FASTA != formatId) {
        setError(tr("The detected reference sequence format is '%1'. Only FASTA is supported").arg(formatId));
        return;
    }
}

const GUrl& PrepareToImportTask::getSourceUrl() const {
    return sourceURL;
}

bool PrepareToImportTask::isNewURL() {
    return newURL;
}

} // namespace BAM
} // namespace U2