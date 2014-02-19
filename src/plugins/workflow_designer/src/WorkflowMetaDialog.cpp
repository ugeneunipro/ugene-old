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

#include "WorkflowMetaDialog.h"

#include <U2Lang/WorkflowUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Designer/DesignerUtils.h>
#include <QtGui/QFileDialog>
#include <U2Gui/HelpButton.h>


namespace U2 {

WorkflowMetaDialog::WorkflowMetaDialog(QWidget * p, const Metadata& meta): QDialog(p), meta(meta) {
    setupUi(this);
    new HelpButton(this, buttonBox, "2097175");

    cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    okButton = buttonBox->button(QDialogButtonBox::Ok);

    connect(browseButton, SIGNAL(clicked()), SLOT(sl_onBrowse()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
    connect(okButton, SIGNAL(clicked()), SLOT(sl_onSave()));
    connect(urlEdit, SIGNAL(textChanged(const QString & )), SLOT(sl_onURLChanged(const QString& )));
    connect(urlEdit, SIGNAL(textEdited (const QString & )), SLOT(sl_onURLChanged(const QString& )));

    urlEdit->setText(meta.url);
    okButton->setDisabled(meta.url.isEmpty());
    nameEdit->setText(meta.name);
    commentEdit->setText(meta.comment);
}

void WorkflowMetaDialog::sl_onSave() {
    assert(!WorkflowUtils::WD_FILE_EXTENSIONS.isEmpty());
    QString url = urlEdit->text();
    bool endsWithWDExt = false;
    foreach( const QString & ext, WorkflowUtils::WD_FILE_EXTENSIONS ) {
        assert(!ext.isEmpty());
        if(url.endsWith(ext)) {
            endsWithWDExt = true;
        }
    }
    if(!endsWithWDExt) {
        url.append("." + WorkflowUtils::WD_FILE_EXTENSIONS.first());
    }
    meta.url = url;
    meta.comment = commentEdit->toPlainText();
    meta.name = nameEdit->text();
    accept();
}

void WorkflowMetaDialog::sl_onURLChanged(const QString & text) {
    okButton->setDisabled(text.isEmpty());
}

#define LAST_DIR QString("workflowview/lastdir")

void WorkflowMetaDialog::sl_onBrowse() {
    QString url = urlEdit->text();
    if (url.isEmpty()) {
        url = AppContext::getSettings()->getValue(LAST_DIR, QString("")).toString();
    }
    QString filter = DesignerUtils::getSchemaFileFilter(false);
    url = QFileDialog::getSaveFileName(0, tr("Save workflow to file"), url, filter);
    if (!url.isEmpty()) {
        AppContext::getSettings()->setValue(LAST_DIR, QFileInfo(url).absoluteDir().absolutePath());
        urlEdit->setText(url);
    }
}


}//namespace
