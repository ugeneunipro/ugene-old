#include "AddExistingDocumentDialogImpl.h"

#include "DocumentFormatComboboxController.h"

#include <U2Core/L10n.h>
#include <U2Core/Settings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentFormatConfigurators.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Formats/DocumentFormatUtils.h>

#include <U2Misc/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <memory>

/* TRANSLATOR U2::AddExistingDocumentDialogImpl */

namespace U2 {


#define SETTINGS_LAST_FORMAT	    "add_existing_document/last_format"
#define SETTINGS_LAST_FILE_FILTER	"add_existing_document/last_file_filter"

    AddExistingDocumentDialogImpl::AddExistingDocumentDialogImpl(QWidget* p,  AddExistingDocumentDialogModel& m) 
        : QDialog(p), model(m), filter(NULL)
    {
        setupUi(this);

        if (model.format.isEmpty()) {
            model.format = BaseDocumentFormats::PLAIN_TEXT;
        }

        documentURLEdit->setText(model.url);
        forceReadOnlyCheck->setChecked(model.readOnly);
        save2DiskCheck->setDisabled( true );

        updateAvailableFormats();

        model.successful = false;

        connect(documentURLButton, SIGNAL(clicked()), SLOT(sl_documentURLButtonClicked()));
        connect(formatSettingsButton, SIGNAL(clicked()), SLOT(sl_customFormatSettingsClicked()));
        connect(addButton, SIGNAL(clicked()), SLOT(sl_addButtonClicked()));
        connect(documentURLEdit, SIGNAL(editingFinished()), SLOT(sl_documentURLEditFinished()));
        connect(documentURLEdit, SIGNAL(textEdited(const QString&)), SLOT(sl_documentURLTextEdited(const QString&)));
        connect(forceFormatCombo, SIGNAL(currentIndexChanged(int)), SLOT(sl_formatComboCurrentChanged(int)));

        updateState();
        setWindowIcon(QIcon(":/ugene/images/ugene_16.png"));
    }


    void AddExistingDocumentDialogController::run(QWidget* p, AddExistingDocumentDialogModel& m, bool runFileDialog ) {
        m.successful = false;
        Project* proj = AppContext::getProject();
        if (proj->isStateLocked()) {
            QMessageBox::critical(NULL, L10N::errorTitle(), 
                AddExistingDocumentDialogImpl::tr("Project is locked"));
            return;
        }
        if (runFileDialog) {
            m.url = AddExistingDocumentDialogImpl::selectFile(p);
            if (m.url.isEmpty()) {
                return;
            }
        }

        AddExistingDocumentDialogImpl d(p, m);
        d.exec();
        m = d.model;
        assert(!m.successful || proj->findDocumentByURL(m.url) == NULL);
    }

    QString AddExistingDocumentDialogImpl::selectFile(QWidget* p) {
        QString filter = DialogUtils::prepareDocumentsFileFilter(true);
        QString allDocsFilter = filter.split('\n').first();
        QString selectedFilter = AppContext::getSettings()->getValue(SETTINGS_LAST_FILE_FILTER, allDocsFilter).toString();
        LastOpenDirHelper lod;
        QFileDialog d(p, tr("Open file"), lod.dir, filter);
        d.setFileMode(QFileDialog::ExistingFile);
        d.setWindowIcon(QIcon(":/ugene/images/ugene_16.png"));
        d.selectNameFilter(selectedFilter);
        if (QDialog::Rejected == d.exec()) {
            return lod.url;
        }
        QStringList files = d.selectedFiles();
        if (!files.isEmpty()) {
            lod.url = files.at(0);
        }

        if (!lod.url.isEmpty()) {
            AppContext::getSettings()->setValue(SETTINGS_LAST_FILE_FILTER, selectedFilter);
        }
        return lod.url;
    }

    static bool isHyperLink( const QString & str ) {
        return str.startsWith("http://") || str.startsWith("https://");
    }

    void AddExistingDocumentDialogImpl::updateState() {
        const QString& url = documentURLEdit->text();
        bool isNetLink = isHyperLink(url);
        DocumentFormatId format = DocumentFormatComboboxController::getActiveFormatId(forceFormatCombo);
        formatSettingsButton->setEnabled(AppContext::getDocumentFormatConfigurators()->findConfigurator(format)!=NULL);
        save2DiskCheck->setEnabled(isNetLink);
    }

    void AddExistingDocumentDialogImpl::updateAvailableFormats() {
        QString url = documentURLEdit->text();
        DocumentFormatConstraints c;
        if (isHyperLink(url) || url.isEmpty()) {
            DocumentFormatId activeFormat = model.format;
            if (activeFormat.isEmpty()) {
                activeFormat = AppContext::getSettings()->getValue(SETTINGS_LAST_FORMAT, BaseDocumentFormats::PLAIN_GENBANK).toString();
            }
            QList<DocumentFormatId> allFormats = AppContext::getDocumentFormatRegistry()->getRegisteredFormats();
            DocumentFormatComboboxController::fill(forceFormatCombo, allFormats, activeFormat);
            return;
        }

        QList<DocumentFormatId> detectedFormats = DocumentFormatUtils::toIds(DocumentUtils::detectFormat(url));
        DocumentFormatId activeFormat = detectedFormats.isEmpty() ? DocumentFormatId() : detectedFormats.first();
        DocumentFormatComboboxController::fill(forceFormatCombo, detectedFormats, activeFormat);
    }

    void AddExistingDocumentDialogImpl::sl_documentURLButtonClicked() {
        QString file = selectFile(this);
        if (!file.isEmpty()) {
            documentURLEdit->setText(file);	
            updateAvailableFormats();
            updateState();
        }
    }

    void AddExistingDocumentDialogImpl::sl_addButtonClicked() {
        QString url = documentURLEdit->text();
        bool isNetLink = isHyperLink(url);

        U2::GUrl gurl = GUrl(url);
        if(gurl.getType() == GUrl_File) {
            QFileInfo fi(gurl.getURLString());
            if(!fi.exists()) {
                QMessageBox::critical(this, L10N::errorTitle(), L10N::errorFileNotFound(gurl));
                documentURLEdit->setFocus();
                return;
            }

            if(!fi.isFile()) {
                QMessageBox::critical(this, L10N::errorTitle(), L10N::errorIsNotAFile(gurl));
                documentURLEdit->setFocus();
                return;
            }
        }

        if (url.isEmpty()) {
            QMessageBox::critical(this, tr("Error!"), tr("Document URL is empty!"));
            documentURLEdit->setFocus();
            return;
        }
        Project* p = AppContext::getProject();
        if (p->findDocumentByURL(url) != NULL) {
            QMessageBox::critical(this, tr("Error!"), tr("Document is already added!"));
            documentURLEdit->setFocus();
            return;
        }
        DocumentFormatId format = DocumentFormatComboboxController::getActiveFormatId(forceFormatCombo);
        if (format.isEmpty()) {
            QMessageBox::critical(this, tr("Error!"), tr("Document format is not selected!"));
            forceFormatCombo->setFocus();
            return;
        }
        IOAdapterId io = BaseIOAdapters::url2io(url);
        if (io.isEmpty()) {
            QMessageBox::critical(this, tr("Error!"), tr("IO adapter not found!"));
            documentURLEdit->setFocus();
            return;
        }

        if ( save2DiskCheck->isChecked() ) { //copy-paste from above
            QString filter = DialogUtils::prepareDocumentsFileFilter(true);
            QString allDocsFilter = filter.split('\n').first();
            QString selectedFilter = AppContext::getSettings()->getValue(SETTINGS_LAST_FILE_FILTER, allDocsFilter).toString();

            LastOpenDirHelper lod;
            lod.url = model.dwnldPath = QFileDialog::getSaveFileName( this, tr("Save as"), lod, allDocsFilter, &selectedFilter );
        }
        model.format = format;
        model.url = url;
        model.io = io;
        model.successful = true;

        QFileInfo f(model.url);
        model.readOnly = forceReadOnlyCheck->isChecked() || isNetLink || !f.isWritable();

        AppContext::getSettings()->setValue(SETTINGS_LAST_FORMAT, model.format);

        accept();
    }


    void AddExistingDocumentDialogImpl::sl_documentURLTextEdited(const QString&) {
        bool isNetLink = isHyperLink(documentURLEdit->text());
        save2DiskCheck->setEnabled( isNetLink);
    }

    void AddExistingDocumentDialogImpl::sl_documentURLEditFinished() {
        updateState();
        updateAvailableFormats();
    }

    void AddExistingDocumentDialogImpl::sl_formatComboCurrentChanged(int i) {
        Q_UNUSED(i);
        model.formatSettings.clear();
        updateState();
    }


    void AddExistingDocumentDialogImpl::sl_customFormatSettingsClicked() {
        DocumentFormatId format =  DocumentFormatComboboxController::getActiveFormatId(forceFormatCombo);
        DocumentFormatConfigurator* c = AppContext::getDocumentFormatConfigurators()->findConfigurator(format);
        if (c == NULL) {
            return;
        }
        c->configure(model.formatSettings);
    }

}//namespace
