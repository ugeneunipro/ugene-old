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

#pragma once

#include <ui/ui_TreeSettingsDialog.h>
#include "ov_phyltree/TreeSettings.h"
#include <QtGui/QDialog>

namespace U2 {

class TreeSettingsDialog : public QDialog, public Ui_TreeSettingsDialog{
    Q_OBJECT
public:
    TreeSettingsDialog(QWidget *parent, const TreeSettings &treeSettings, bool isRectLayout);
    
    virtual void accept();
    const TreeSettings& getSettings() const;

    static QString treeDefaultText() { return tr("Default"); }
    static QString treePhylogramText() { return tr("Phylogram"); }
    static QString treeCladogramText() { return tr("Cladogram"); }

protected slots:
   // void sl_colorButton();

private:

    TreeSettings settings;
    TreeSettings changedSettings;

};

} //namespace


