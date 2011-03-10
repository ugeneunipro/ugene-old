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

