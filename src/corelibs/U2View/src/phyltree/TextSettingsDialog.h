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

#ifndef _U2_PHYLTREE_TEXT_SETTINGS_DIALOG_H_
#define _U2_PHYLTREE_TEXT_SETTINGS_DIALOG_H_

#include <ui/ui_TextSettingsDialog.h>
#include <QtGui/QFontInfo>
#include <QtGui/QDialog>

namespace U2 {

class TextSettings {
public:
    TextSettings();

    QColor textColor;
    QFont textFont;

    static QColor defaultColor;
    static QFont defaultFont;
};

class TextSettingsDialog : public QDialog, public Ui_TextSettingsDialog{
    Q_OBJECT
public:
    TextSettingsDialog(QWidget *parent, const TextSettings &textSettings);

    virtual void accept();
    TextSettings getSettings() const;

protected slots:
    void sl_colorButton();

private:
    TextSettings settings, changedSettings;

    void updateColorButton();
};

} //namespace

#endif
