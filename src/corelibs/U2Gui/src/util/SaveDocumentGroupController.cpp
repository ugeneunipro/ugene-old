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

#include "SaveDocumentGroupController.h"

#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/DocumentFormatComboboxController.h>

#include <QtGui/QFileDialog>

namespace U2 {

SaveDocumentGroupController::SaveDocumentGroupController(const SaveDocumentGroupControllerConfig& _conf, QObject* _parent)
: QObject(_parent)
{
	conf = _conf;
    if (conf.saveTitle.isEmpty()) {
        conf.saveTitle = tr("Select file location");
    }
    assert(conf.parentWidget!=NULL);

	conf.fileNameEdit->setText(conf.defaultFileName);
    comboController = new DocumentFormatComboboxController(this, conf.formatCombo, conf.dfc, conf.defaultFormatId);

	connect(conf.fileNameEdit, SIGNAL(textChanged(const QString&)), SLOT(sl_fileNameChanged(const QString&)));
	connect(conf.fileDialogButton, SIGNAL(clicked()), SLOT(sl_saveButtonClicked()));
    connect(comboController->comboBox(), SIGNAL(currentIndexChanged ( const QString& )), SLOT(sl_formatChanged(const QString&)));

}

void SaveDocumentGroupController::sl_fileNameChanged(const QString& newName) {
	GUrl url(newName);
	QString ext = GUrlUtils::getUncompressedExtension(url);
	DocumentFormatRegistry* formatRegistry = AppContext::getDocumentFormatRegistry();
	DocumentFormat* df = formatRegistry->selectFormatByFileExtension(ext);
	if (df!=NULL && df->checkConstraints(conf.dfc)) {
		comboController->setActiveFormatId(df->getFormatId());
	}
}

DocumentFormatId SaveDocumentGroupController::getFormatIdToSave() const {
	return comboController->getActiveFormatId();
}

void SaveDocumentGroupController::setSelectedFormatId(DocumentFormatId id) {
    comboController->setActiveFormatId(id);
}

DocumentFormat* SaveDocumentGroupController::getFormatToSave() const {
	DocumentFormatId id = getFormatIdToSave();
	if (id.isEmpty()) {
		return NULL;
	}
	return AppContext::getDocumentFormatRegistry()->getFormatById(id);
}

void SaveDocumentGroupController::sl_saveButtonClicked() {
    //prepare filters
    QString filter = DialogUtils::prepareDocumentsFileFilter(conf.dfc, false);

    // find the default one
    QStringList extList = getFormatToSave()->getSupportedDocumentFileExtensions();
    QString selectedFilterExt = "*." + extList.first();
    QString selectedFilter;
    foreach (QString filterLine, filter.split(";;")) {
        if (filterLine.contains(selectedFilterExt)) {
            selectedFilter = filterLine;
            break;
        }
    }

    // show the dialog
	LastUsedDirHelper lod;
	lod.url = QFileDialog::getSaveFileName(conf.parentWidget, conf.saveTitle, lod, filter, &selectedFilter);
	if (lod.url.isEmpty()) {
		return;
	}
    QFileInfo f(lod.url);
    if(!extList.contains(f.suffix())){
        lod.url.append("." + extList.first());
    }
	conf.fileNameEdit->setText(lod.url);
}

void SaveDocumentGroupController::sl_formatChanged( const QString& )
{
    if (conf.fileNameEdit->text().isEmpty()) {
        return;
    }

    DocumentFormatId id = comboController->getActiveFormatId();
    QString fileExt = AppContext::getDocumentFormatRegistry()->getFormatById(id)->getSupportedDocumentFileExtensions().first();
    GUrl url =  conf.fileNameEdit->text();
    conf.fileNameEdit->setText(QString("%1/%2.%3").arg(url.dirPath()).arg(url.baseFileName()).arg(fileExt));
}

} //namespace
