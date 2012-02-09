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

#include "AttributeRelation.h"
#include <U2Core/GUrl.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>

namespace U2 {

QVariant VisibilityRelation::getAffectResult(const QVariant &influencingValue, const QVariant &) const {
    return influencingValue == visibilityValue;
}

QVariant FileExtensionRelation::getAffectResult(const QVariant &influencingValue, const QVariant &dependentValue) const {
    QString urlStr = dependentValue.toString();
    if (urlStr.isEmpty()) {
        return "";
    }
    QString newFormatId = influencingValue.toString();
    GUrl url(urlStr);

    DocumentFormat *currentFormat = AppContext::getDocumentFormatRegistry()->getFormatById(currentFormatId);
    DocumentFormat *newFormat = AppContext::getDocumentFormatRegistry()->getFormatById(newFormatId);
    QString extension;
    if (NULL == newFormat) {
        extension = newFormatId;
    } else {
        extension = newFormat->getSupportedDocumentFileExtensions().first();
    }
    QString urlString = url.getURLString();
    QString lastSuffix = url.lastFileSuffix();
    bool withGz = false;

    if ("gz" == lastSuffix) {
        int dotPos = urlString.length() - lastSuffix.length() - 1;
        if ((dotPos >= 0) && (QChar('.') == urlString[dotPos])) {
            withGz = true;
            urlString = url.getURLString().left(dotPos);
            GUrl tmp(urlString);
            lastSuffix = tmp.lastFileSuffix(); 
        }
    }

    bool foundExt = false;
    if (NULL == currentFormat) {
        foundExt = (lastSuffix == currentFormatId);
    } else {
        QStringList extensions(currentFormat->getSupportedDocumentFileExtensions());
        if (NULL == newFormat) {
            extensions << newFormatId;
        } else {
            extensions << newFormat->getSupportedDocumentFileExtensions();
        }
        foreach (QString supExt, extensions) {
            if (lastSuffix == supExt) {
                foundExt = true;
                break;
            }
        }
    }
    
    if (foundExt) {
        int dotPos = urlString.length() - lastSuffix.length() - 1;
        if ((dotPos >= 0) && (QChar('.') == urlString[dotPos])) { //yes, lastSuffix is a correct extension with .
            urlString = url.getURLString().left(dotPos);
        }
    }

    const_cast<QString&>(currentFormatId).clear();
    const_cast<QString&>(currentFormatId).append(newFormatId);
    urlString += "." + extension;
    if (withGz) {
        urlString += ".gz";
    }
    return urlString;
}

} // U2
