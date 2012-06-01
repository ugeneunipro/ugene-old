/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef __U2_BASE_TYPES_H_
#define __U2_BASE_TYPES_H_

#include <U2Lang/Datatype.h>

namespace U2 {

class U2LANG_EXPORT BaseTypes : public QObject {
    Q_OBJECT
public:
    static DataTypePtr DNA_SEQUENCE_TYPE();
    static DataTypePtr ANNOTATION_TABLE_LIST_TYPE();
    static DataTypePtr ANNOTATION_TABLE_TYPE();
    static DataTypePtr MULTIPLE_ALIGNMENT_TYPE();
    static DataTypePtr VARIATION_TRACK_TYPE();
    
    static DataTypePtr STRING_TYPE();
    static DataTypePtr STRING_LIST_TYPE();
    static DataTypePtr BOOL_TYPE();
    static DataTypePtr NUM_TYPE();
    static DataTypePtr ANY_TYPE();
    
}; // BaseTypes

// value factories for core datatypes
/**
* DatatypeValueFactory realization for string type
*/
class StringTypeValueFactory : public DataTypeValueFactory {
public:
    virtual QVariant getValueFromString( const QString & str, bool * ok = NULL  ) const;
    virtual QString getId() const { return BaseTypes::STRING_TYPE()->getId(); }

}; // StringTypeValueFactory

/**
* DatatypeValueFactory realization for bool type
*/
class BoolTypeValueFactory : public DataTypeValueFactory {
public:
    static const QString TRUE_STR;
    static const QString FALSE_STR;
    static const QString YES_STR;
    static const QString NO_STR;
    static const QString ONE_STR;
    static const QString NIL_STR;

public:
    virtual QVariant getValueFromString( const QString & str, bool * ok = NULL  ) const;
    virtual QString getId() const { return BaseTypes::BOOL_TYPE()->getId(); }

}; // BoolTypeValueFactory

/**
* DatatypeValueFactory realization for num type
*/
class NumTypeValueFactory : public DataTypeValueFactory {
public:
    virtual QVariant getValueFromString( const QString & str, bool * ok = NULL ) const;
    virtual QString getId() const { return BaseTypes::NUM_TYPE()->getId(); }

}; // NumTypeValueFactory

}

#endif // __U2_BASE_TYPES_H_
