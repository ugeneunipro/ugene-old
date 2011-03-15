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

#ifndef _U2_ADD_EXISTING_DOCUMENT_DIALOG_IMPL_H_
#define _U2_ADD_EXISTING_DOCUMENT_DIALOG_IMPL_H_

#include "AddExistingDocumentDialogController.h"
#include <ui/ui_AddExistingDocumentDialog.h>

namespace U2 {

class SupportedFormatsFilter;

class AddExistingDocumentDialogImpl : public QDialog, public Ui::AddExistingDocumentDialog {
	Q_OBJECT
friend class AddExistingDocumentDialogController;
	AddExistingDocumentDialogImpl(QWidget* p,  AddExistingDocumentDialogModel& m);

private slots:
	void sl_documentURLButtonClicked();
	void sl_addButtonClicked();
	void sl_customFormatSettingsClicked();
	void sl_documentURLEditFinished();
    void sl_documentURLTextEdited(const QString&);
	void sl_formatComboCurrentChanged(int i);

private:
    static QString selectFile(QWidget* p);

	void updateState();
	void updateAvailableFormats();

    AddExistingDocumentDialogModel model;
	SupportedFormatsFilter* filter;

};

} //namespace

#endif

