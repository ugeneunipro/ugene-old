#ifndef _U2_COPY_DOCUMENT_DIALOG_CONTROLLER_H_
#define _U2_COPY_DOCUMENT_DIALOG_CONTROLLER_H_

#include <U2Core/global.h>
#include <U2Core/DocumentModel.h>
#include <QtGui/QDialog>

class Ui_CopyDocumentDialog;

namespace U2 {

class SaveDocumentGroupController;

class U2GUI_EXPORT CopyDocumentDialogController : public QDialog {
    Q_OBJECT
public:
    CopyDocumentDialogController(Document* d, QWidget* p);
    ~CopyDocumentDialogController();

    QString getDocumentURL() const;
    
    bool getAddToProjectFlag() const;

    DocumentFormatId getDocumentFormatId() const;

private:
    SaveDocumentGroupController* saveController;
    Ui_CopyDocumentDialog* ui;
};

}//ns

#endif
