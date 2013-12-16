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

#ifndef _U2_UDRSCHEMAREGISTRY_H_
#define _U2_UDRSCHEMAREGISTRY_H_

#include <QtCore/QMutex>

#include <U2Core/UdrSchema.h>

namespace U2 {

class U2CORE_EXPORT UdrSchemaRegistry {
    Q_DISABLE_COPY(UdrSchemaRegistry)
public:
    UdrSchemaRegistry();
    ~UdrSchemaRegistry();

    void registerSchema(const UdrSchema *schema, U2OpStatus &os);
    QList<UdrSchemaId> getRegisteredSchemas();
    const UdrSchema * getSchemaById(const UdrSchemaId &id);

    /**
     * Check the names of schemas and fields that they consist of
     * Latin letters, digits (not first character) and "_" only.
     */
    static bool isCorrectName(const QByteArray &name);

private:
    QMutex mutex;
    QHash<UdrSchemaId, const UdrSchema*> schemas;
};

} // U2

#endif // _U2_UDRSCHEMAREGISTRY_H_
