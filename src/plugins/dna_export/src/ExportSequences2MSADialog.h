#ifndef _U2_EXPORT_SEQ2MSA_DIALOG_H_
#define _U2_EXPORT_SEQ2MSA_DIALOG_H_

#include <U2Core/global.h>

#include <QtGui/QDialog>
#include <ui/ui_ExportSequences2MSADialog.h>

namespace U2 {

class SaveDocumentGroupController;

class ExportSequences2MSADialog : public QDialog, Ui_ExportSequences2MSADialog {
    Q_OBJECT
public:
    ExportSequences2MSADialog(QWidget* p, const QString& defaultUrl = QString() );
    
    void setOkButtonText(const QString& text) const;
    void setFileLabelText(const QString& text) const;

    virtual void accept();


public:    
    QString url;
    DocumentFormatId format;
    bool    addToProjectFlag;

private:    
    SaveDocumentGroupController* saveContoller;

};

}//namespace

#endif
