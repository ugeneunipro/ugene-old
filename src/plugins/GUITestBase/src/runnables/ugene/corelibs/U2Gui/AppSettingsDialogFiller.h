/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <base_dialogs/GTFileDialog.h>

namespace U2{
using namespace HI;

class AppSettingsDialogFiller: public Filler
{
public:
    enum style{minimal,extended,none};
    enum Tabs{General, Resourses, Network, FileFormat, Directories, Logging, AlignmentColorScheme, GenomeAligner,
             WorkflowDesigner, ExternalTools, OpenCL};
    AppSettingsDialogFiller(HI::GUITestOpStatus &_os, style _itemStyle=extended):Filler(_os,"AppSettingsDialog"),
        itemStyle(_itemStyle),r(-1),g(-1),b(-1){}
    AppSettingsDialogFiller(HI::GUITestOpStatus &_os, int _r,int _g, int _b):Filler(_os,"AppSettingsDialog"),
        itemStyle(none),r(_r),g(_g),b(_b){}
    AppSettingsDialogFiller(HI::GUITestOpStatus &os, CustomScenario *customScenario);
    void commonScenario();

    static void setExternalToolPath(HI::GUITestOpStatus &os, const QString& toolName, const QString& toolPath);
    static QString getExternalToolPath(HI::GUITestOpStatus &os, const QString& toolName);
    static void openTab(HI::GUITestOpStatus &os, Tabs tab);
    static bool isExternalToolValid(HI::GUITestOpStatus &os, const QString& toolName);
    static void clearToolPath(HI::GUITestOpStatus &os, const QString& toolName);
private:
    style itemStyle;
    int r,g,b;
    static const QMap<Tabs, QString> tabMap;
    static QMap<Tabs, QString> initMap();
};

class NewColorSchemeCreator: public Filler{
public:
    enum alphabet{amino,nucl};
    enum Action{Create, Delete};
    NewColorSchemeCreator(HI::GUITestOpStatus &_os, QString _schemeName, alphabet _al, Action _act = Create, bool cancel = false);
    NewColorSchemeCreator(HI::GUITestOpStatus &os, CustomScenario* c);
    virtual void commonScenario();

private:
    QString schemeName;
    alphabet al;
    Action act;
    bool cancel;
};

class CreateAlignmentColorSchemeDialogFiller: public Filler{
public:

    CreateAlignmentColorSchemeDialogFiller(HI::GUITestOpStatus &os, QString _schemeName, NewColorSchemeCreator::alphabet _al):
        Filler(os, "CreateMSAScheme"), schemeName(_schemeName), al(_al){}
    CreateAlignmentColorSchemeDialogFiller(HI::GUITestOpStatus &os, CustomScenario *c): Filler(os, "CreateMSAScheme", c){}
    virtual void commonScenario();
private:
    QString schemeName;
    NewColorSchemeCreator::alphabet al;
};

class ColorSchemeDialogFiller: public Filler{
public:
    ColorSchemeDialogFiller(HI::GUITestOpStatus &os): Filler(os, "ColorSchemaDialog"){}
    ColorSchemeDialogFiller(HI::GUITestOpStatus &os, CustomScenario *c): Filler(os, "ColorSchemaDialog", c){}
    virtual void commonScenario();
};
}
#endif // APPSETTINGSDIALOGFILLER_H
