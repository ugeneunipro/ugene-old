#include "SaveDocumentGroupController.h"

#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>

#include <U2Misc/DialogUtils.h>
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
    QString selectedFilterExt = "*." + getFormatToSave()->getSupportedDocumentFileExtensions().first();
    QString selectedFilter;
    foreach (QString filterLine, filter.split(";;")) {
        if (filterLine.contains(selectedFilterExt)) {
            selectedFilter = filterLine;
            break;
        }
    }

    // show the dialog
	LastOpenDirHelper lod;
	lod.url = QFileDialog::getSaveFileName(conf.parentWidget, conf.saveTitle, lod, filter, &selectedFilter);
	if (lod.url.isEmpty()) {
		return;
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
