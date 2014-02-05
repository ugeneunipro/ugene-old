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

#include "TreeSettings.h"
#include "ov_phyltree/TreeViewerUtils.h"


//TreeSettings
namespace U2 {
int TreeSettings::default_width_coef = 1;
int TreeSettings::default_height_coef = 1;

TreeSettings::TreeSettings() :
    type(DEFAULT),
    width_coef(default_width_coef),
    height_coef(default_height_coef)
{
}

bool TreeSettings::operator ==(const TreeSettings &other) const {
    return width_coef == other.width_coef && 
        height_coef == other.height_coef;
}

//TextSettings
QColor TextSettings::defaultColor = QColor(Qt::gray);
QFont TextSettings::defaultFont = TreeViewerUtils::getFont();

TextSettings::TextSettings() :
    textColor(defaultColor),
    textFont(defaultFont)
{
}

bool TextSettings::operator ==(const TextSettings &other) const {
    return textColor == other.textColor && 
        textFont == other.textFont;
}

//BranchSettings
QColor BranchSettings::defaultColor = QColor(0, 0, 0);
int BranchSettings::defaultThickness = 1;

BranchSettings::BranchSettings() :
    branchColor(defaultColor),
    branchThickness(defaultThickness)
{
}

bool BranchSettings::operator ==(const BranchSettings &other) const {
    return branchColor == other.branchColor && 
        branchThickness == other.branchThickness;
}

//ButtonSettings
QColor ButtonSettings::defaultColor = QColor(0, 0, 0);
int ButtonSettings::defaultRadius = 2;

ButtonSettings::ButtonSettings() :
    col(defaultColor), 
    radius(defaultRadius)
{
}

bool ButtonSettings::operator ==(const ButtonSettings &other) const {
    return col == other.col && 
        radius == other.radius;
}

//TreeLabelsSettings
bool TreeLabelsSettings::operator ==(const TreeLabelsSettings &other) const {
    return showNames == other.showNames && 
        showDistances == other.showDistances &&
        alignLabels == other.alignLabels;
}
} //namespace