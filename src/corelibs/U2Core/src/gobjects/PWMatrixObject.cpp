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

#include <U2Core/DatatypeSerializeUtils.h>
#include <U2Core/RawDataUdrSchema.h>

#include "../util/PMatrixSerializeUtils.h"
#include "PWMatrixObject.h"

namespace U2 {

const GObjectType PWMatrixObject::TYPE("pwm-obj");

//PWMatrixObject
//////////////////////////////////////////////////////////////////////////

U2PWMatrix::U2PWMatrix() : U2RawData() {

}

U2PWMatrix::U2PWMatrix(const U2DbiRef &dbiRef) : U2RawData(dbiRef) {

}

U2DataType U2PWMatrix::getType() {
    return U2Type::PWMatrix;
}

//PWMatrixObject
//////////////////////////////////////////////////////////////////////////
PWMatrixObject * PWMatrixObject::createInstance(const PWMatrix &matrix, const QString &objectName,
    const U2DbiRef &dbiRef, U2OpStatus &os, const QVariantMap &hintsMap)
{
    U2PWMatrix object(dbiRef);
    const U2EntityRef entRef = PMatrixSerializeUtils<WMatrixSerializer, PWMatrix>::commit(matrix,
        objectName, dbiRef, object, os);
    CHECK_OP(os, NULL);
    return new PWMatrixObject(matrix, objectName, entRef, hintsMap);
}

PWMatrixObject::PWMatrixObject(const QString &objectName, const U2EntityRef &matrixRef,
    const QVariantMap &hintsMap)
    : GObject(TYPE, objectName, hintsMap)
{
    entityRef = matrixRef;
}

PWMatrixObject::PWMatrixObject(const PWMatrix &matrix, const QString &objectName,
    const U2EntityRef &matrixRef, const QVariantMap &hintsMap)
    : GObject(TYPE, objectName, hintsMap), m(matrix)
{
    entityRef = matrixRef;
}

void PWMatrixObject::loadDataCore(U2OpStatus &os) {
    PMatrixSerializeUtils<WMatrixSerializer, PWMatrix>::retrieve(entityRef, m, os);
}

const PWMatrix & PWMatrixObject::getMatrix() const {
    ensureDataLoaded();
    return m;
}

GObject * PWMatrixObject::clone(const U2DbiRef &dstRef, U2OpStatus &os) const {
    U2PWMatrix dstObject;
    RawDataUdrSchema::cloneObject(entityRef, dstRef, dstObject, os);
    CHECK_OP(os, NULL);

    const U2EntityRef dstEntRef(dstRef, dstObject.id);
    PWMatrixObject *dst = new PWMatrixObject(getGObjectName(), dstEntRef, getGHintsMap());
    dst->setIndexInfo(getIndexInfo());
    return dst;
}

} // namespace U2
