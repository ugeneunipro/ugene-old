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

#ifndef _U2_STRUCTURAL_ALIGNMENT_DIALOG_H_
#define _U2_STRUCTURAL_ALIGNMENT_DIALOG_H_

#include "ui_StructuralAlignmentDialog.h"

namespace U2 {

class BioStruct3DObject;


class StructuralAlignmentDialog : public QDialog, public Ui::StructuralAlignmentDialog
{
    Q_OBJECT

public:
    StructuralAlignmentDialog(const QList<BioStruct3DObject*> biostructs, const BioStruct3DObject *fixedRef = 0, int fixedRefModel = 0, QWidget *parent = 0);

public slots:
    virtual void accept();

private slots:
    virtual void sl_biostructChanged(int idx);
    
private:
    static void createModelList(QComboBox *biostruct, int idx, QComboBox *model);
    void createModelLists();
};

}   // namespace U2

#endif  // #ifndef _U2_STRUCTURAL_ALIGNMENT_DIALOG_H_
