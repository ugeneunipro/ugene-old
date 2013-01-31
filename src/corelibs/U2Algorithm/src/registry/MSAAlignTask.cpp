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

#include "MSAAlignTask.h"

namespace U2 {

MSAAlignTask::MSAAlignTask(MAlignmentObject* _obj, const MSAAlignTaskSettings& s, TaskFlags _flags)
 : Task("MSAAlignTask", _flags), obj(_obj), settings(s) {}


QVariant MSAAlignTaskSettings::getCustomValue( const QString& optionName, const QVariant& defaultVal ) const
{
    if (customSettings.contains(optionName)) {
        return customSettings.value(optionName);
    } else {
        return defaultVal;
    }    
}

void MSAAlignTaskSettings::setCustomValue( const QString& optionName, const QVariant& val )
{
    customSettings.insert(optionName,val);
}

void MSAAlignTaskSettings::setCustomSettings( const QMap<QString, QVariant>& settings )
{
    customSettings = settings;
}
} // U2

