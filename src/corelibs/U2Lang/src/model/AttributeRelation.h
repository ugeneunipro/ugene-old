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

#ifndef _U2_ATTRIBUTE_RELATION_H_
#define _U2_ATTRIBUTE_RELATION_H_

#include <U2Core/global.h>

namespace U2 {

enum RelationType {
    VISIBILITY
};

/**
 * An abstract class describing any relations between influencing and dependent attributes
 * It should be contained by the dependent attribute
 */
class U2LANG_EXPORT AttributeRelation {
public:
    AttributeRelation(const QString &influencingAttrName, const QVariant &attrValue)
        : influencingAttrName(influencingAttrName), attrValue(attrValue) {}

    virtual QVariant getAffectResult(const QVariantMap &values) const = 0;
    virtual RelationType getType() const = 0;

    virtual ~AttributeRelation() {}

protected:
    QString influencingAttrName;
    QVariant attrValue;
};

/**
 * It describes any attribute's visibility depends on a value of another attribute
 */
class U2LANG_EXPORT VisibilityRelation : public AttributeRelation {
public:
    VisibilityRelation(const QString &influencingAttrName, const QVariant &attrValue)
        : AttributeRelation(influencingAttrName, attrValue) {}

    virtual QVariant getAffectResult(const QVariantMap &values) const;
    virtual RelationType getType() const {return VISIBILITY;}
};

} // U2 namespace

#endif // _U2_ATTRIBUTE_RELATION_H_
