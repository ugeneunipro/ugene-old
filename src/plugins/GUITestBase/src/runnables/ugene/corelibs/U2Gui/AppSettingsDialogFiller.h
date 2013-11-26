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
#ifndef APPSETTINGSDIALOGFILLER_H
#define APPSETTINGSDIALOGFILLER_H

#include "api/GTFileDialog.h"

namespace U2{

class AppSettingsDialogFiller: public Filler
{
public:
    enum style{minimal,extended,none};
    AppSettingsDialogFiller(U2OpStatus &_os, style _itemStyle=extended):Filler(_os,"AppSettingsDialog"),
        itemStyle(_itemStyle),r(-1),g(-1),b(-1){}
    AppSettingsDialogFiller(U2OpStatus &_os, int _r,int _g, int _b):Filler(_os,"AppSettingsDialog"),
        itemStyle(none),r(_r),g(_g),b(_b){}
    void run();
private:
    style itemStyle;
    int r,g,b;
};

class NewColorSchemeCreator: public Filler{
public:
    enum alphabet{amino,nucl};
    NewColorSchemeCreator(U2OpStatus &_os, QString _schemeName, alphabet _al): Filler(_os,"AppSettingsDialog"),
        schemeName(_schemeName), al(_al){}
    virtual void run();
private:
    QString schemeName;
    alphabet al;
};

class CreateAlignmentColorSchemeDialogFiller: public Filler{
public:

    CreateAlignmentColorSchemeDialogFiller(U2OpStatus &os, QString _schemeName, NewColorSchemeCreator::alphabet _al):
        Filler(os, "CreateMSAScheme"), schemeName(_schemeName), al(_al){}
    virtual void run();
private:
    QString schemeName;
    NewColorSchemeCreator::alphabet al;
};

class ColorSchemeDialogFiller: public Filler{
public:
    ColorSchemeDialogFiller(U2OpStatus &os): Filler(os, "ColorSchemaDialog"){}
    virtual void run();
};
}
#endif // APPSETTINGSDIALOGFILLER_H
