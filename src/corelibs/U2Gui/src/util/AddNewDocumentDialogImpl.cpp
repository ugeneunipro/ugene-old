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

#include "AddNewDocumentDialogImpl.h"
#include "DocumentFormatComboboxController.h"

#include <U2Gui/GUIUtils.h>
#include <U2Gui/DialogUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/Settings.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/AppContext.h>

#include <U2Gui/HelpButton.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPushButton>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#endif


namespace U2 {

//BUG:419: add label to dialog with state description!

#define SETTINGS_LASTFORMAT     "add_new_document/last_format"
#define SETTINGS_LASTDIR        "add_new_document/last_dir"

AddNewDocumentDialogImpl::AddNewDocumentDialogImpl(QWidget* p, AddNewDocumentDialogModel& m, const DocumentFormatConstraints& c) 
: QDialog(p), model(m)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "4227404");
    if (model.format.isEmpty()) {
        model.format = AppContext::getSettings()->getValue(SETTINGS_LASTFORMAT, QString("")).toString();
    }
    
    documentURLEdit->setText(model.url);
    formatController = new DocumentFormatComboboxController(this, documentTypeCombo, c, model.format);
    model.successful = false;
    
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Create"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connect(documentURLButton, SIGNAL(clicked()), SLOT(sl_documentURLButtonClicked()));
    QPushButton *createButton = buttonBox->button(QDialogButtonBox::Ok);
    connect(createButton, SIGNAL(clicked()), SLOT(sl_createButtonClicked()));
    connect(documentURLEdit, SIGNAL(editingFinished()), SLOT(sl_documentURLEdited()));
    connect(documentTypeCombo, SIGNAL(currentIndexChanged(int)), SLOT(sl_typeComboCurrentChanged(int)));
    connect(gzipCheckBox, SIGNAL(toggled(bool)), SLOT(sl_gzipChecked(bool)));

    updateState();
}

void AddNewDocumentDialogController::run(QWidget* p, AddNewDocumentDialogModel& m, const DocumentFormatConstraints& c) {
    Project* proj = AppContext::getProject();
    if (proj->isStateLocked()) {
        QMessageBox::critical(NULL, L10N::errorTitle(), AddNewDocumentDialogImpl::tr("Project is locked"));
        m.successful = false;
        return;
    }

    AddNewDocumentDialogImpl d(p, m, c);
    d.exec();
    m = d.model;
    assert(proj->findDocumentByURL(m.url) == NULL);
}

void AddNewDocumentDialogImpl::updateState() {
    bool ready = formatController->hasSelectedFormat();
    
    if (ready) {
        const QString& url = currentURL();
        ready = !url.isEmpty() && QFileInfo(url).absoluteDir().exists();
        if (ready) {
            Project* p = AppContext::getProject();
            ready = p->findDocumentByURL(url) == NULL;
        }
    }
    
    //createButton->setDisabled(!ready);
}


QString AddNewDocumentDialogImpl::currentURL() {
	QString url = documentURLEdit->text();
	if (url.isEmpty()) {
		return url;
	}
	bool extOk = false;
	QString extraExt = ".gz";
	if (url.endsWith(extraExt)) {
		url.chop(extraExt.size());
	}

	DocumentFormatId fid = formatController->getActiveFormatId();
	DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(fid);
	if (df) {
		foreach(QString ext, df->getSupportedDocumentFileExtensions()) {
			if (url.endsWith(ext)) {
				extOk = true;
				break;
			}
		}
		if (!extOk) {
			url+="."+df->getSupportedDocumentFileExtensions().first();
		}
	}
	if (gzipCheckBox->isChecked()) {
		url += extraExt;
	}
	documentURLEdit->setText(url);
	
	QFileInfo fi(url);
	return fi.absoluteFilePath();
}

void AddNewDocumentDialogImpl::sl_documentURLButtonClicked() {
	QString url = currentURL();
    if (url.isEmpty()) {
		url = AppContext::getSettings()->getValue(SETTINGS_LASTDIR, QString("")).toString();
    }
    QString filter = DialogUtils::prepareDocumentsFileFilter(formatController->getActiveFormatId(), false);
	QString name = QFileDialog::getSaveFileName(this, tr("Save file"), url, filter);
	if (!name.isEmpty()) {
		documentURLEdit->setText(name);	
		AppContext::getSettings()->setValue(SETTINGS_LASTDIR, QFileInfo(name).absoluteDir().absolutePath());
		updateState();
	}
}

void AddNewDocumentDialogImpl::sl_createButtonClicked() {
	model.format = formatController->getActiveFormatId();
	assert(!model.format.isEmpty());
	model.url = currentURL();
	model.io = gzipCheckBox->isChecked() ? BaseIOAdapters::GZIPPED_LOCAL_FILE : BaseIOAdapters::LOCAL_FILE;
	model.successful = true;
	AppContext::getSettings()->setValue(SETTINGS_LASTFORMAT, model.format);
	accept();
}



void AddNewDocumentDialogImpl::sl_documentURLEdited() {
	updateState();
}

void AddNewDocumentDialogImpl::sl_typeComboCurrentChanged(int i) {
    Q_UNUSED(i);
	QString url = documentURLEdit->text();
	if (!url.isEmpty()) {
		/* chop obsolete extensions */
		if (gzipCheckBox->isChecked() && url.endsWith(".gz")) {
			url.chop(3);
		}
		int dot = url.lastIndexOf('.');
		if (dot > 0) {
			url.chop(url.size() - dot);
		}
		documentURLEdit->setText(url);
	}

	updateState();
}

void AddNewDocumentDialogImpl::sl_gzipChecked(bool on) {
    Q_UNUSED(on);
	updateState();
}

}//namespace
