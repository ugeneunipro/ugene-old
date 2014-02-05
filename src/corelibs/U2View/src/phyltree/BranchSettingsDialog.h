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

#ifndef _U2_PHYLTREE_BRANCH_SETTINGS_DIALOG_H_
#define _U2_PHYLTREE_BRANCH_SETTINGS_DIALOG_H_

#include "ov_phyltree/TreeSettings.h"
#include <QtGui/QDialog>
#include <ui/ui_BranchSettingsDialog.h>

namespace U2 {

class BranchSettingsDialog : public QDialog, public Ui_BranchSettingsDialog{
    Q_OBJECT
public:
    BranchSettingsDialog(QWidget *parent, const BranchSettings& branchSettings);

    virtual void accept();
    const BranchSettings& getSettings() const;

protected slots:
    void sl_colorButton();

private:
    BranchSettings settings;
    BranchSettings changedSettings;

    void updateColorButton();
};

} //namespace

#endif
