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

#include <U2Lang/MarkerAttribute.h>

namespace U2 {

MarkerAttribute::MarkerAttribute(const Descriptor& d, const DataTypePtr type, bool required, const QVariant & defaultValue)
: Attribute(d, type, required, defaultValue)
{

}

void MarkerAttribute::setAttributeValue(const QVariant &newVal) {
    value = newVal;
    QString line = newVal.toString();
    QStringList markerIds = line.split(",");

    for (int i = 0; i < markerIds.size(); i++) {
        markers.insert(markerIds.at(i).trimmed(), NULL);
    }
}

const QVariant &MarkerAttribute::getAttributePureValue() const {
    QString result;
    bool first = true;

    foreach (QString markerId, markers.keys()) {
        if (!first) {
            result += ", ";
        }
        result += markerId;
        first = false;
    }
    const_cast<QVariant&>(value) = qVariantFromValue(result);
    return value;
}

bool MarkerAttribute::isDefaultValue() const {
    return (defaultValue == getAttributePureValue());
}

Attribute *MarkerAttribute::clone() {
    return new MarkerAttribute(*this);
}

AttributeGroup MarkerAttribute::getGroup() {
    return MARKER_GROUP;
}

QMap<QString, Marker*> &MarkerAttribute::getMarkers() {
    return markers;
}

} //U2
