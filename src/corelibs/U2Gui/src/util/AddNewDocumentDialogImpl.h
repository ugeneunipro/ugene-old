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

#ifndef _U2_ADD_NEW_DOCUMENT_DIALOG_IMPL_H_
#define _U2_ADD_NEW_DOCUMENT_DIALOG_IMPL_H_

#include "AddNewDocumentDialogController.h"
#include <ui/ui_AddNewDocumentDialog.h>

namespace U2 {

class DocumentFormatComboboxController;

class AddNewDocumentDialogImpl : public QDialog, public Ui::AddNewDocumentDialog {
    Q_OBJECT
    AddNewDocumentDialogImpl(QWidget* p, AddNewDocumentDialogModel& m, const DocumentFormatConstraints& c);
    friend class AddNewDocumentDialogController;

private slots:
    void sl_documentURLButtonClicked();
    void sl_createButtonClicked();
    void sl_documentURLEdited();
    void sl_typeComboCurrentChanged(int i);
    void sl_gzipChecked(bool state);

private:
    void updateState();
    QString currentURL();
    

    AddNewDocumentDialogModel model;
    DocumentFormatComboboxController* formatController;
};


}//namespace

#endif
