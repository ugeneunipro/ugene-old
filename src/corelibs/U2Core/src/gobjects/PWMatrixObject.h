/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PW_MATRIX_OBJECT_H_
#define _U2_PW_MATRIX_OBJECT_H_

#include <U2Core/GObject.h>
#include <U2Core/PWMatrix.h>
#include <U2Core/U2RawData.h>

namespace U2 {

class U2CORE_EXPORT U2PWMatrix : public U2RawData {
public:
                    U2PWMatrix();
                    U2PWMatrix(const U2DbiRef &dbiRef);

    U2DataType      getType();
};

class U2CORE_EXPORT PWMatrixObject : public GObject {
    Q_OBJECT
public :
    static const GObjectType TYPE;

                                PWMatrixObject(const QString &objectName,
                                    const U2EntityRef &matrixRef,
                                    const QVariantMap &hintsMap = QVariantMap());

    static PWMatrixObject *     createInstance(const PWMatrix &matrix, const QString &objectName,
                                    const U2DbiRef &dbiRef, U2OpStatus &os,
                                    const QVariantMap &hintsMap = QVariantMap());

    const PWMatrix &            getMatrix() const;
    GObject *                   clone(const U2DbiRef &dstDbiRef, U2OpStatus &os, const QVariantMap &hints = QVariantMap()) const;

protected :
    void                        loadDataCore(U2OpStatus &os);

private :
                                PWMatrixObject(const PWMatrix &matrix, const QString &objectName,
                                    const U2EntityRef &matrixRef, const QVariantMap &hintsMap);

    PWMatrix                    m;
};

} // namespace U2

#endif // _U2_PW_MATRIX_OBJECT_H_
