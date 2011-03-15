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

#ifndef _CREATEPHYTREEDIALOGCONTROLLER_H_
#define _CREATEPHYTREEDIALOGCONTROLLER_H_

#include <U2Core/global.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>

#include <QtGui/QDialog>

class Ui_CreatePhyTree;
class QWidget;

namespace U2{

class MAlignmentObject;
class MAlignment;
class CreatePhyTreeWidget;

class U2VIEW_EXPORT CreatePhyTreeDialogController : public QDialog {
    Q_OBJECT
public:
    CreatePhyTreeDialogController(QWidget* parent, const MAlignmentObject* mobj, CreatePhyTreeSettings& settings);
    ~CreatePhyTreeDialogController();

    void insertWidget(int pos, CreatePhyTreeWidget* widget);

private slots:
    void sl_okClicked();
    void sl_browseClicked();
    
private:
    int rightMargin;
    const MAlignment& msa;
    CreatePhyTreeSettings& settings;
    QList<CreatePhyTreeWidget*> childWidgets;
    Ui_CreatePhyTree* ui;

    bool estimateResources(); 
};

}

#endif