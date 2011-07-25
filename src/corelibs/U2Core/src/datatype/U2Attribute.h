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

#ifndef _U2_ATTRIBUTE_H_
#define _U2_ATTRIBUTE_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Region.h>

#include <QtCore/QDateTime>
#include <QtCore/QVector>

namespace U2 {


/**
    Attribute for any top-level object record.
    Some name/value are supported internally and must be tracked by the system on sequence modification
*/
class U2CORE_EXPORT U2Attribute : public U2Entity {
public:
    U2Attribute() : version(0) {}
    
    /** parent object id */
    U2DataId    objectId;

    /** Additional object id, optional */
    U2DataId    childId;

    /** parent object version this attribute is related to. If <=0 -> any is OK. */
    qint64      version;
    
    /* Name of the attribute */
    QString     name;
};

/** 64 bit signed integer attribute */
class U2CORE_EXPORT U2IntegerAttribute : public U2Attribute {
public:
    U2IntegerAttribute() : value(0) {}
    qint64 value;
};

/** 64 bit real attribute */
class U2CORE_EXPORT U2RealAttribute : public U2Attribute {
public:
    U2RealAttribute() : value(0.0) {}
    double value;
};

/** String attribute */
class U2CORE_EXPORT U2StringAttribute : public U2Attribute {
public:
    QString value;
};

/** Byte array attribute */
class U2CORE_EXPORT U2ByteArrayAttribute: public U2Attribute {
public:
    QByteArray value;
};

} //namespace

#endif
