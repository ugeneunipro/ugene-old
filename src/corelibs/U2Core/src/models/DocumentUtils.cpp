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

#include "DocumentUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentImport.h>

namespace U2 {

QSet<QString> DocumentUtils::getURLs(const QList<Document*>& docs) {
    QSet<QString> result;
    foreach(Document* d, docs) {
        result.insert(d->getURLString());
    }
    return result;
}

QSet<QString> DocumentUtils::getNewDocFileNameExcludesHint() {
    QSet<QString> excludeFileNames;
    Project* p = AppContext::getProject();
    if (p!=NULL) {
        excludeFileNames = DocumentUtils::getURLs(p->getDocuments());
    }
    return excludeFileNames;
    
}


static void placeOrderedByScore(const FormatDetectionResult& info, QList<FormatDetectionResult>& result, const FormatDetectionConfig& conf) {
    if (info.score() == FormatDetection_NotMatched) {
        return;
    }
    if (result.isEmpty()) {
        result.append(info);
        return;
    }
    if (conf.bestMatchesOnly) {
        int bestScore = result.first().score();
        if (bestScore > info.score()) {
            return;
        } else if (bestScore < info.score()) {
            result.clear();
        } 
        result.append(info);
        return;
    }
    for (int i = 0; i < result.length(); i++) {
        int scoreI = result.at(i).score();
        if (scoreI < info.score()) {
            result.insert(i, info);
            return;
        }
    }
    result.append(info);
}

#define FORMAT_DETECTION_EXT_BONUS 3

// returns formats with FormatDetectionResult != Not matched sorted by FormatDetectionResult
// FormatDetectionResult is adjusted by +FORMAT_DETECTION_EXT_BONUS if extension is matched

QList<FormatDetectionResult> DocumentUtils::detectFormat( const QByteArray& rawData, const QString& ext, 
                                                     const GUrl& url, const FormatDetectionConfig& conf) 
{
    DocumentFormatRegistry* fr = AppContext::getDocumentFormatRegistry();
    QList< DocumentFormatId > allFormats = fr->getRegisteredFormats();

    QList<FormatDetectionResult> result;
    foreach(const DocumentFormatId& id, allFormats) {
        DocumentFormat* f = fr->getFormatById(id);
        FormatCheckResult cr = f->checkRawData(rawData, url);
        if (cr.score ==  FormatDetection_NotMatched) {
            continue;
        }
        if (conf.useExtensionBonus && f->getSupportedDocumentFileExtensions().contains(ext) && cr.score >= FormatDetection_VeryLowSimilarity) {
            cr.score += FORMAT_DETECTION_EXT_BONUS;
        }
        FormatDetectionResult res;
        res.format = f;
        res.rawDataCheckResult = cr;
        res.rawData = rawData;
        res.url = url;
        res.extension = ext;
        placeOrderedByScore(res, result, conf);
    }
    if (conf.useImporters) {
        DocumentImportersRegistry* importReg = AppContext::getDocumentFormatRegistry()->getImportSupport();
        foreach(DocumentImporter* i, importReg->getImporters()) {
            FormatCheckResult cr = i->checkRawData(rawData, url);
            if (conf.useExtensionBonus && i->getSupportedFileExtensions().contains(ext) && cr.score >= FormatDetection_VeryLowSimilarity) {
                cr.score += FORMAT_DETECTION_EXT_BONUS;
            }
            FormatDetectionResult res;
            res.importer = i;
            res.rawDataCheckResult = cr;
            res.rawData = rawData;
            res.url = url;
            res.extension = ext;
            placeOrderedByScore(res, result, conf);
        }
    }
    return result;
}

QList<FormatDetectionResult> DocumentUtils::detectFormat(const GUrl& url, const FormatDetectionConfig& conf) {
    QList<FormatDetectionResult> result;
    if( url.isEmpty() ) {
        return result;
    }
    QByteArray rawData = BaseIOAdapters::readFileHeader(url);
    if (rawData.isEmpty()) {
        return result;
    }
    QString ext = GUrlUtils::getUncompressedExtension(url);
    result = detectFormat(rawData, ext, url, conf);
    return result;
}


QList<FormatDetectionResult> DocumentUtils::detectFormat(IOAdapter *io, const FormatDetectionConfig& conf) {
    QList<FormatDetectionResult> result;
    if (io == NULL || !io->isOpen()) {
        return result;
    }
    QByteArray rawData = BaseIOAdapters::readFileHeader( io );
    QString ext = GUrlUtils::getUncompressedExtension(io->getURL());
    result = detectFormat( rawData, ext , io->getURL(), conf);
    return result;
}

QList<DocumentFormat*> DocumentUtils::toFormats(const QList<FormatDetectionResult>& infos) {
    QList<DocumentFormat*> result;
    foreach(const FormatDetectionResult& info, infos) {
        if (info.format != NULL) {
            result << info.format;
        }
    }
    return result;
}

bool DocumentUtils::canAddGObjectsToDocument( Document* doc, const GObjectType& type )
{
    if (!doc->isLoaded() || doc->isStateLocked()) {
        return false;
    }

    DocumentFormat* df = doc->getDocumentFormat();
    return df->isObjectOpSupported(doc, DocumentFormat::DocObjectOp_Add, type);
}

bool DocumentUtils::canRemoveGObjectFromDocument( GObject* obj )
{
    Document* doc = obj->getDocument();

    if (!doc->isLoaded() || doc->isStateLocked()) {
        return false;
    }

    if (doc->getObjects().size() < 2) {
        // cannot remove the only object in the document
        return false;
    }

    DocumentFormat* df = doc->getDocumentFormat();
    if (!df->isObjectOpSupported(doc, DocumentFormat::DocObjectOp_Remove, obj->getGObjectType() )) {
        return false;
    }
    
    return true;   
}

QString FormatDetectionResult::getFormatDescriptionText() const {
    QString text = format == NULL ? importer->getImporterDescription() : format->getFormatDescription();
    return text;
}

QString FormatDetectionResult::getFormatOrImporterName() const {
    QString name = format == NULL ? importer->getImporterName() : format->getFormatName();
    return name;
}

} //namespace
