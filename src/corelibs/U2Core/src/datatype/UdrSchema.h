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

#ifndef _U2_UDRSCHEMA_H_
#define _U2_UDRSCHEMA_H_

#include <U2Core/U2OpStatus.h>

namespace U2 {

typedef QByteArray UdrSchemaId;

/**
 * The descriptive class for keeping Unified Data Record schemas.
 * It consists of Unified Data Record fields and multi indexes descriptions.
 */
class U2CORE_EXPORT UdrSchema {
public:
    /**
     * Types of field's data.
     */
    enum U2CORE_EXPORT DataType {
        INTEGER,
        DOUBLE,
        STRING,
        BLOB
    };

    /**
     * Fields indexing can be used for optimizing access to the data.
     */
    enum U2CORE_EXPORT IndexType {
        INDEXED,
        NOT_INDEXED
    };

    /**
     * The auxiliary descriptive class for keeping Unified Data Record fields.
     */
    class U2CORE_EXPORT FieldDesc {
    public:
        FieldDesc(const QByteArray &name, DataType dataType, IndexType indexType = NOT_INDEXED);

        /**
         * The name of the field.
         * It must consist of Latin letters, digits (not first character) and "_" only.
         */
        const QByteArray name;
        /**
         * The type of the field's data.
         */
        const DataType dataType;
        /**
         * Indexed field or not.
         */
        const IndexType indexType;
    };

public:
    /**
     * The schema identifier is unique within UGENE.
     * It must consist of Latin letters, digits (not first character) and "_" only.
     */
    UdrSchema(const UdrSchemaId &id);

    /**
     * The name of a new field must be unique within the schema.
     * The number of a field in the order of adding is the field identifier (fieldNum).
     */
    void addField(const FieldDesc &desc, U2OpStatus &os);

    /**
     * A multi index is a unique list of fieldNums.
     * The index for fields with these fieldNums will be created
     * for optimized access to the data.
     */
    void addMultiIndex(const QList<int> &multiIndex, U2OpStatus &os);

    /**
     * Returns the list of multi indexes.
     */
    const QList< QList<int> > & getMultiIndexes() const;

    /**
     * Returns the schema identifier.
     */
    const UdrSchemaId & getId() const;

    /**
     * Return the count of fields.
     */
    int size() const;

    /**
     * Returns the field description of the specified number.
     */
    FieldDesc getField(int fieldNum, U2OpStatus &os) const;

    /**
     * Reserved name of the field for record identifier.
     */
    static const QByteArray RECORD_ID_FIELD_NAME;

private:
    const UdrSchemaId id;
    QList< QList<int> > multiIndexes;
    QList<FieldDesc> fields;

private:
    bool contains(const QByteArray &name) const;
};

} // U2

#endif // _U2_UDRSCHEMA_H_
