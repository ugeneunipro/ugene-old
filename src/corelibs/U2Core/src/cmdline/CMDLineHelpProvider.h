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

#ifndef _U2_CMD_LINE_HELP_PROVIDER_
#define _U2_CMD_LINE_HELP_PROVIDER_

#include "U2Core/global.h"

namespace U2 {

class U2CORE_EXPORT CMDLineHelpProvider {
public:
    CMDLineHelpProvider( const QString & _fullName, const QString & _content, const QString & _shortName = QString() ) 
        : fullName(_fullName), shortName(_shortName), content(_content) {}
    
    QString getHelpSectionNames() const { QString ret = fullName; if(!shortName.isEmpty()) { ret += ", -" + shortName; } return ret; }
    QString getHelpSectionFullName() const {return fullName;}
    QString getHelpSectionShortName() const {return shortName;}
    QString getHelpSectionContent() const { return content; }
    
private:
    QString fullName;
    QString shortName;
    QString content;
    
}; // CMDLineHelpProvider

}//namespace

#endif
