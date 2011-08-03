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

#ifndef _U2_DOCUMENT_UTILS_H_
#define _U2_DOCUMENT_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/IOAdapter.h>

#include "DocumentModel.h"

namespace U2 {

class DocumentImporter;

/** 
    Result of the format detection.
    contains score and ( format or importer ) instance pointer
*/
class U2CORE_EXPORT FormatDetectionResult {
public:
    FormatDetectionResult() : format(NULL), importer(NULL){}
    DocumentFormat*         format;
    DocumentImporter*       importer;
    QByteArray              rawData;
    GUrl                    url;
    QString                 extension;

    FormatCheckResult      rawDataCheckResult;

    QString getFormatDescriptionText() const;
    QString getFormatOrImporterName() const;
    int score() const {return rawDataCheckResult.score;}

};

class U2CORE_EXPORT FormatDetectionConfig {
public:
    FormatDetectionConfig() : bestMatchesOnly(true), useImporters(false), useExtensionBonus(true){}
    
    /** if true format detection algorithm returns list of best matches only */
    bool bestMatchesOnly;
    
    /** if false format detection algorithm do not test format importers and returns only real formats */
    bool useImporters;

    /** if true file extension is checked and bonus is added if extension is matched for a format*/
    bool useExtensionBonus;
};

class U2CORE_EXPORT DocumentUtils: public QObject    {
public:
    /* returns set with document urls */
    static QSet<QString> getURLs(const QList<Document*>& docs);
        
    /*  The set of urls that should not be used for new documents
        returns list of loaded urls. Gets them from the active project 
    */
    static QSet<QString> getNewDocFileNameExcludesHint();

    /* Detects document format. The best match goes first in the returned list */
    static QList<FormatDetectionResult> detectFormat(const GUrl& url, const FormatDetectionConfig& conf = FormatDetectionConfig());

    /* 
        Detects document format. The best match goes first in the returned list 
        IOAdapter must be opened
    */
    static QList<FormatDetectionResult> detectFormat(IOAdapter* io, const FormatDetectionConfig& conf = FormatDetectionConfig());

    /* 
        Detects document format. The best match goes first in the returned list 
        ext & url can be used here to add extension bonus to the final score
    */
    static QList<FormatDetectionResult> detectFormat(const QByteArray& rawData, const QString& ext = QString(), 
                                            const GUrl& url = GUrl(), const FormatDetectionConfig& conf = FormatDetectionConfig());

    static QList<DocumentFormat*> toFormats(const QList<FormatDetectionResult>& infos);

    static bool canAddGObjectsToDocument(Document* doc, const GObjectType& type);

    static bool canRemoveGObjectFromDocument(GObject* obj);
};

}//namespace

#endif

