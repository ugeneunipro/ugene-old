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

#ifndef _U2_CREATE_PHY_TREE_DIALOG_CONTROLLER_H_
#define _U2_CREATE_PHY_TREE_DIALOG_CONTROLLER_H_

#include <QDialog>

#include <U2Algorithm/CreatePhyTreeSettings.h>

#include <U2Core/global.h>
#include <U2Core/MAlignment.h>

class Ui_CreatePhyTree;

namespace U2 {

class MAlignmentObject;
class CreatePhyTreeWidget;

class U2VIEW_EXPORT CreatePhyTreeDialogController : public QDialog {
    Q_OBJECT
public:
    CreatePhyTreeDialogController(QWidget* parent, const MAlignmentObject* mobj, CreatePhyTreeSettings& settings);
    ~CreatePhyTreeDialogController();

private slots:
    void accept();
    void sl_browseClicked();
    void sl_comboIndexChaged(int index);
    void sl_onStoreSettings();
    void sl_onRestoreDefault();

private:
    bool checkLicense();
    bool checkFileName();
    bool checkSettings();
    bool checkMemory();

    MAlignment msa;
    CreatePhyTreeSettings& settings;
    QList<CreatePhyTreeWidget*> childWidgets;
    CreatePhyTreeWidget *settingsWidget;
    Ui_CreatePhyTree* ui;
};

}   // namespace U2

#endif // _U2_CREATE_PHY_TREE_DIALOG_CONTROLLER_H_
