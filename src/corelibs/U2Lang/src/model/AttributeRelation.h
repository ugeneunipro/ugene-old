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

#ifndef _U2_ATTRIBUTE_RELATION_H_
#define _U2_ATTRIBUTE_RELATION_H_

#include <U2Core/global.h>

namespace U2 {

enum RelationType {
    VISIBILITY,
    FILE_EXTENSION,
    CUSTOM_VALUE_CHANGER
};

/**
 * An abstract class describing any relations between influencing and dependent attributes
 */
class U2LANG_EXPORT AttributeRelation {
public:
    AttributeRelation(const QString &relatedAttrId)
        : relatedAttrId(relatedAttrId) {}

    virtual QVariant getAffectResult(const QVariant &influencingValue, const QVariant &dependentValue) const = 0;
    virtual RelationType getType() const = 0;
    QString getRelatedAttrId() const {return relatedAttrId;}

    virtual ~AttributeRelation() {}

protected:
    QString relatedAttrId;
};

/**
 * It describes how visibility of some attribute depends on a value of the related attribute
 */
class U2LANG_EXPORT VisibilityRelation : public AttributeRelation {
public:
    VisibilityRelation(const QString &relatedAttrId, const QVariantList &visibilityValues);
    VisibilityRelation(const QString &relatedAttrId, const QVariant &visibilityValue);

    virtual QVariant getAffectResult(const QVariant &influencingValue, const QVariant &dependentValue) const;
    virtual RelationType getType() const {return VISIBILITY;}

private:
    QVariantList visibilityValues;
};

/**
 * A value of URL_OUT_ATTRIBUTE depends on a value of DOCUMENT_FORMAT_ATTRIBUTE
 */
class U2LANG_EXPORT FileExtensionRelation : public AttributeRelation {
public:
    FileExtensionRelation(const QString &relatedAttrId)
        : AttributeRelation(relatedAttrId){}

    virtual QVariant getAffectResult(const QVariant &influencingValue, const QVariant &dependentValue) const;
    virtual RelationType getType() const {return FILE_EXTENSION;}
};

} // U2 namespace

#endif // _U2_ATTRIBUTE_RELATION_H_
