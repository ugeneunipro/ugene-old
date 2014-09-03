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

#ifndef _U2_CREATE_PHY_TREE_DIALOG_CONTROLLER_H_
#define _U2_CREATE_PHY_TREE_DIALOG_CONTROLLER_H_

#include <U2Algorithm/CreatePhyTreeSettings.h>

#include <U2Core/global.h>
#include <U2Core/MAlignment.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif


class Ui_CreatePhyTree;
class QWidget;

namespace U2{

class MAlignmentObject;
class CreatePhyTreeWidget;

class U2VIEW_EXPORT CreatePhyTreeDialogController : public QDialog {
    Q_OBJECT
public:
    CreatePhyTreeDialogController(QWidget* parent, const MAlignmentObject* mobj, CreatePhyTreeSettings& settings);
    ~CreatePhyTreeDialogController();

    void insertContrWidget(int pos, CreatePhyTreeWidget* widget);
    void clearContrWidgets();

private slots:
    void sl_okClicked();
    void sl_browseClicked();
    void sl_comboIndexChaged(int index);
    void sl_onStoreSettings();
    void sl_onRestoreDefault();
    void sl_onDispayWithMSAClicked(bool checked);

private:
    int rightMargin;
    MAlignment msa;
    CreatePhyTreeSettings& settings;
    QList<CreatePhyTreeWidget*> childWidgets;
    Ui_CreatePhyTree* ui;
};

}

#endif
