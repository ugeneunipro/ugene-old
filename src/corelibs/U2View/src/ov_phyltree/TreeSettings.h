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

#ifndef _U2_TREE_SETTINGS_H_
#define _U2_TREE_SETTINGS_H_

#include <QtGui/QFontInfo>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

namespace U2 {

enum TreeSettingsType {
    GENERAL_SETTINGS, 
    TEXT_FORMAT,
    BRANCHES_SETTINGS,
    BUTTONS_SETTINGS,
    LABELS_SETTINGS
};

class TreeSettings
{
public:
    TreeSettings();

    enum TREE_TYPE { PHYLOGRAM, CLADOGRAM, DEFAULT };

    TREE_TYPE type;
    int width_coef;
    int height_coef;

    static int default_width_coef;
    static int default_height_coef;

    bool operator==(const TreeSettings &other) const;
    inline bool operator !=(const TreeSettings &other) const { return !operator==(other); }
};

class TextSettings {
public:
    TextSettings();

    QColor textColor;
    QFont textFont;

    static QColor defaultColor;
    static QFont defaultFont;

    bool operator==(const TextSettings &other) const;
    inline bool operator !=(const TextSettings &other) const { return !operator==(other); }
};

class BranchSettings {
public:
    BranchSettings();

    QColor branchColor;
    int branchThickness;

    static QColor defaultColor;
    static int defaultThickness;

    bool operator==(const BranchSettings &other) const;
    inline bool operator !=(const BranchSettings &other) const { return !operator==(other); }
};

class ButtonSettings {
public:
    ButtonSettings();

    QColor col;
    int radius;

    static QColor defaultColor;
    static int defaultRadius;

    bool operator==(const ButtonSettings &other) const;
    inline bool operator !=(const ButtonSettings &other) const { return !operator==(other); }
};

class TreeLabelsSettings {
public:
    TreeLabelsSettings()
        : showNames(true), showDistances(true), alignLabels(false) {}

    bool showNames;
    bool showDistances;
    bool alignLabels;

    bool operator==(const TreeLabelsSettings &other) const;
    inline bool operator !=(const TreeLabelsSettings &other) const { return !operator==(other); }
};

} //namespace
#endif
