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


#ifndef _U2_UINDEX_H_
#define _U2_UINDEX_H_

#include <QObject>
#include <QString>
#include <QHash>

#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT UIndex  {
public:
    //errors
    enum UIndexError {
        NO_ERR,
        EMPTY_IO_ID,
        EMPTY_DOC_FORMAT,
        EMPTY_IO_ADAPTER,
        EMPTY_URL,
        BAD_OFFSET,
        NO_SUCH_IO,
        EMPTY_KEY_VAL
    };
    
    struct U2CORE_EXPORT ItemSection {
        QString                  ioSectionId;
        DocumentFormatId         docFormat;
        qint64                   startOff;
        qint64                   endOff;
        QHash<QString, QString > keys;

        UIndexError checkConsistentcy() const;
    };

    struct U2CORE_EXPORT IOSection {
        QString                   sectionId;
        IOAdapterId               ioAdapterId;
        QString                   url;
        QHash< QString, QString > keys;

        UIndexError checkConsistentcy() const;
    };
    
    IOSection getIOSection(const QString& id) const;
    bool hasItems() const;
    
    QList< ItemSection > items;
    QList< IOSection   > ios;
    
    UIndexError checkConsistency() const;
}; // UIndex

} // U2

#endif // _U2_UINDEX_H_
