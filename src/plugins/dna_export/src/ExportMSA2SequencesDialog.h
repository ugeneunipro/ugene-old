#ifndef _U2_EXPORT_MSA2SEQ_DIALOG_H_
#define _U2_EXPORT_MSA2SEQ_DIALOG_H_

#include <U2Core/global.h>

#include <QtGui/QDialog>
#include <ui/ui_ExportMSA2SequencesDialog.h>

namespace U2 {

class SaveDocumentGroupController;

class ExportMSA2SequencesDialog : public QDialog, Ui_ExportMSA2SequencesDialog {
    Q_OBJECT
public:
    ExportMSA2SequencesDialog(QWidget* p);

    virtual void accept();

public:
    QString url;
    DocumentFormatId format;
    bool    trimGapsFlag;
    bool    addToProjectFlag;

private:    
    SaveDocumentGroupController* saveContoller;
};

}//namespace

#endif
