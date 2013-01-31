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

#include <U2Core/U2AnnotationUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

QStringList U2AnnotationUtils::splitPath(const QString& groupPath, U2OpStatus& os) {
    CHECK_EXT(!groupPath.isEmpty(), os.setError(tr("Group path is empty!")), QStringList());
    
    QStringList res = groupPath.split(GROUP_PATH_SEPARATOR, QString::KeepEmptyParts);
    CHECK_EXT(!res.contains(""), os.setError(tr("Not a valid group path: %1").arg(groupPath)), QStringList());

    return res;
}

QString U2AnnotationUtils::getNameFromPath(const QString& groupPath, U2OpStatus& os) {
    QStringList tokens = splitPath(groupPath, os);
    CHECK_OP(os, QString());
    return tokens.last();
}

} //namespace

