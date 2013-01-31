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


#include "Peak2GeneSettings.h"

#include <U2Core/GUrlUtils.h>

namespace U2 {

const QString Peak2GeneSettings::OUT_TYPE_UPSTREAM = "up";
const QString Peak2GeneSettings::OUT_TYPE_DOWNSTREAM = "down";
const QString Peak2GeneSettings::OUT_TYPE_ALL = "all";

Peak2GeneSettings::Peak2GeneSettings() {
    initDefault();
}

void Peak2GeneSettings::initDefault(){
    outDir = "";
    fileNames = "Default";
    outpos = Peak2GeneSettings::OUT_TYPE_UPSTREAM;
    symbol = false;
    distance = 3000;
    genomePath = "";
}

QStringList Peak2GeneSettings::getArguments( const QString& treatFilePath){
    QStringList result;

    result << "--treat=" + GUrlUtils::getQuotedString(treatFilePath);

    result << "--name=" + fileNames;

    result << "--op=" + outpos;

    if (symbol){
        result << "--symbol";    
    }

    result << "--distance=" + QByteArray::number(distance);

    result << "--genome=" + GUrlUtils::getQuotedString(genomePath);

    return result;
}

} // U2
