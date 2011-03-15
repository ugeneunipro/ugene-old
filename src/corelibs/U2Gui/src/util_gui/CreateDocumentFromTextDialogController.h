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

#ifndef _U2_CREATE_DOCUMENT_FROM_TEXT_DIALOG_CONTROLLER_H_
#define _U2_CREATE_DOCUMENT_FROM_TEXT_DIALOG_CONTROLLER_H_

#include "SeqPasterWidgetController.h"

#include <U2Core/global.h>
#include <U2Core/DocumentModel.h>
#include <U2Misc/DialogUtils.h>

#include <QtGui/QDialog>

class Ui_CreateDocumentFromTextDialog;

namespace U2 {

class U2GUI_EXPORT CreateDocumentFromTextDialogController : public QDialog {
    Q_OBJECT
public:
    CreateDocumentFromTextDialogController(QWidget* p = NULL);
    ~CreateDocumentFromTextDialogController();

    virtual void accept();
    virtual void reject();
    Document* getDocument(){return doc;};

private slots:
    void sl_browseButtonClicked();
    void sl_indexChanged(int index);
    void sl_projectLoaded();
    
private:
    void addSeqPasterWidget();
    void acceptWithExistingProject();
    
    Document *doc;
    SeqPasterWidgetController *w;
    QString filter;
    Ui_CreateDocumentFromTextDialog* ui;
};

}//ns

#endif