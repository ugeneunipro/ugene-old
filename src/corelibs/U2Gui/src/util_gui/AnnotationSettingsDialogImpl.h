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

#ifndef _U2_ANNOTATION_SETTINGS_DIALOG_IMPL_H_
#define _U2_ANNOTATION_SETTINGS_DIALOG_IMPL_H_

#include <ui/ui_AnnotationSettingsDialog.h>
#include <U2Core/AnnotationSettings.h>

#include <QtGui/QDialog>
#include <QtGui/QTreeWidgetItem>

namespace U2 {

class AnnotationSettingsDialogImpl : public QDialog, public Ui_AnnotationSettingsDialog {
Q_OBJECT
public:
    AnnotationSettingsDialogImpl(QWidget* p);

private slots:
    void sl_itemClicked(QTreeWidgetItem * item, int column);
    void sl_okClicked();
    void sl_itemDoubleClicked(QTreeWidgetItem* item, int col);

private:
    void loadModel();
    void storeModel();
};


//////////////////////////////////////////////////////////////////////////
class ASTreeItem : public QTreeWidgetItem {
public:
    ASTreeItem(const AnnotationSettings* as);
    
    void fillModel();
    
    void drawColorCell();

    AnnotationSettings as;
};

}//namespace

#endif
