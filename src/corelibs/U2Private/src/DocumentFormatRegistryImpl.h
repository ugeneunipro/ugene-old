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

#ifndef _DOCUMENT_FORMAT_REGISTRY_IMPL_H_
#define _DOCUMENT_FORMAT_REGISTRY_IMPL_H_

#include "private.h"
#include <U2Core/DocumentModel.h>

namespace U2 {

class U2PRIVATE_EXPORT DocumentFormatRegistryImpl  : public DocumentFormatRegistry {
public:
    DocumentFormatRegistryImpl(QObject* p = NULL) : DocumentFormatRegistry(p) {init();}

    virtual bool registerFormat(DocumentFormat* dfs);

    virtual bool unregisterFormat(DocumentFormat* dfs);

    virtual QList<DocumentFormatId> getRegisteredFormats() const;

    virtual DocumentFormat* getFormatById(DocumentFormatId id) const;

    virtual DocumentFormat* selectFormatByFileExtension(const QString& fileExt) const;

    virtual QList<DocumentFormatId> selectFormats(const DocumentFormatConstraints& c) const;

private:
    void init();
    QList<DocumentFormat*>      formats;
};

}//namespace
#endif
