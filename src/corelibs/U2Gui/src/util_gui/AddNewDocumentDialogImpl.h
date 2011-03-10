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
