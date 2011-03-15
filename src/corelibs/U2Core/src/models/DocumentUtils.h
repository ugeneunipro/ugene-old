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

class U2CORE_EXPORT DocumentUtils: public QObject    {
public:
    /* returns set with document urls */
    static QSet<QString> getURLs(const QList<Document*>& docs);
        
    /*  The set of urls that should not be used for new documents
        returns list of loaded urls. Gets them from the active project 
    */
    static QSet<QString> getNewDocFileNameExcludesHint();

    // the best match goes first in the returned list
    static QList<DocumentFormat*> detectFormat(const GUrl& url);

    // io - opened io adapter
    static QList<DocumentFormat*> detectFormat( IOAdapter* io );

    static QList<DocumentFormat*> detectFormat(const QByteArray& rawData, const QString& ext = QString(), const GUrl& url = GUrl());
};

}//namespace

#endif

