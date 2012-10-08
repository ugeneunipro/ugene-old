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

#include <U2Core/U2SafePoints.h>

#include <U2Lang/URLContainer.h>
#include <U2Lang/WorkflowUtils.h>

#include "URLAttribute.h"

namespace U2 {

URLAttribute::URLAttribute(const Descriptor &d, const DataTypePtr type, bool required, const QVariant &defaultValue)
: Attribute(d, type, required, defaultValue)
{
    sets << Dataset();
    value = qVariantFromValue< QList<Dataset> >(sets);
}

void URLAttribute::setAttributeValue(const QVariant &newVal) {
    if (newVal.canConvert< QList<Dataset> >()) {
        sets = newVal.value< QList<Dataset> >();
    } else {
        QStringList urls = newVal.toString().split(";", QString::SkipEmptyParts);
        Dataset dSet;
        foreach (const QString url, urls) {
            dSet.addUrl(URLContainerFactory::createUrlContainer(url));
        }
        sets.clear();
        sets << dSet;
    }
    value = qVariantFromValue< QList<Dataset> >(sets);
}

const QVariant & URLAttribute::getAttributePureValue() const {
    return value;
}

const QVariant & URLAttribute::getDefaultPureValue() const {
    return defaultValue;
}

bool URLAttribute::isDefaultValue() const {
    return false;
}

Attribute * URLAttribute::clone() {
    URLAttribute *cloned = new URLAttribute(*this, type, required, defaultValue);
    QList<Dataset> sets = value.value< QList<Dataset> >();
    QList<Dataset> clonedSets;
    foreach (const Dataset &dSet, sets) {
        clonedSets << dSet;
    }
    cloned->value = qVariantFromValue< QList<Dataset> >(clonedSets);
    cloned->scriptData = AttributeScript(scriptData);
    return cloned;
}

QList<Dataset> & URLAttribute::getDatasets() {
    return sets;
}

void URLAttribute::updateValue() {
    QList<Dataset> res;
    foreach (const Dataset &dSet, sets) {
        res << dSet;
    }
    value = qVariantFromValue< QList<Dataset> >(res);
}

bool URLAttribute::validate(QStringList &errorList) {
    if (!isRequiredAttribute()) {
        return true;
    }
    if (sets.isEmpty()) {
        errorList << WorkflowUtils::tr("Required parameter has no datasets specified: %1").arg(getDisplayName());
        return false;
    }
    bool hasUrl = false;
    foreach (const Dataset &dSet, sets) {
        if (!dSet.getUrls().isEmpty()) {
            hasUrl = true;
            break;
        }
    }
    if (!hasUrl) {
        errorList << WorkflowUtils::tr("Required parameter has no input urls specified: %1").arg(getDisplayName());
        return false;
    }
    return true;
}

} // U2
