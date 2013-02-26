/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_ADD_PHY_TREE_DIALOG
#define _U2_ADD_PHY_TREE_DIALOG

#include <QtGui/QDialog>
#include "ui/ui_AddPhyTreeDialog.h"

namespace U2 {

class MSAEditor;

class AddTreeDialog: public QDialog, public Ui_AddTreeDialog {
    Q_OBJECT
public:
    AddTreeDialog(QWidget* p, MSAEditor* _msa);
    ~AddTreeDialog(){}

    virtual void accept();
private:
    MSAEditor* msa;
};

}//namespace
#endif
