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

#include "DatasetValidator.h"

#include <U2Lang/Dataset.h>
#include <U2Lang/Attribute.h>
#include <U2Lang/BaseAttributes.h>

namespace U2 {
namespace Workflow {

bool DatasetValidator::validate(const Actor *actor, QStringList &output) const {
    // If parameter is incorrect, return true and skip validation: it is only a warning
    Attribute *urlAttr = actor->getParameter(BaseAttributes::URL_IN_ATTRIBUTE().getId());
    SAFE_POINT(urlAttr != NULL, "Attribute is NULL", true);

    QList<Dataset> sets = urlAttr->getAttributeValueWithoutScript< QList<Dataset> >();
    QScopedPointer<DatasetFilesIterator> filesIt(new DatasetFilesIterator(sets));

    QSet<QString> fileSet;

    while (filesIt->hasNext()) {
        QString fileUrl = filesIt->getNextFile();
        if (!fileSet.contains(fileUrl)) {
            fileSet << fileUrl;
        } else {
            output << QString("Warning: file '%1' was specified several times!").arg(fileUrl);
        }
    }

    return true;
}

}   // namespace Workflow
}   // namespace U2
