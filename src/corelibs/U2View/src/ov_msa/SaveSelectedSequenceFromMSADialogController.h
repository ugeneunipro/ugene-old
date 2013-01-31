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

#ifndef _U2_SAVE_SELECTED_SEQUENCES_DIALOG_CONTROLLER
#define _U2_SAVE_SELECTED_SEQUENCES_DIALOG_CONTROLLER

#include <U2Core/global.h>
#include <U2Gui/SaveDocumentGroupController.h>

#include <QtGui/QDialog>

#include <ui/ui_SaveSelectedSequenceFromMSADialog.h>

namespace U2{

class SaveSelectedSequenceFromMSADialogController : public QDialog, Ui_SaveSelectedSequenceFromMSADialog {
    Q_OBJECT
public:
    SaveSelectedSequenceFromMSADialogController(QWidget* p);

    virtual void accept();

    QString             url;
    DocumentFormatId    format;
    bool                trimGapsFlag;
    bool                addToProjectFlag;

private:    
    SaveDocumentGroupController* saveContoller;
};

}

#endif