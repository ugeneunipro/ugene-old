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

#ifndef _U2_DOC_FORMAT_CONFIG_H_
#define _U2_DOC_FORMAT_CONFIG_H_

#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT DocumentFormatConfigurator : public QObject {
    Q_OBJECT
public:
    DocumentFormatConfigurator(const DocumentFormatId& _formatId, QObject* p = NULL): QObject(p), formatId(_formatId){}
    
    DocumentFormatId getFormatId() const {return formatId;}

    virtual void configure(QVariantMap& info) = 0;

protected:
    DocumentFormatId formatId;
};


class U2CORE_EXPORT DocumentFormatConfigurators : public QObject {
    Q_OBJECT
public:
    DocumentFormatConfigurators(QObject* p = NULL): QObject(p){}
    virtual ~DocumentFormatConfigurators();


    void registerConfigurator(DocumentFormatConfigurator* c);
    
    void unregisterConfigurator(const DocumentFormatId& id);

    DocumentFormatConfigurator* findConfigurator(const DocumentFormatId& id) const;

private:
    QMap<DocumentFormatId, DocumentFormatConfigurator*> configs;
};


}//namespace

#endif
