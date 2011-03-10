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