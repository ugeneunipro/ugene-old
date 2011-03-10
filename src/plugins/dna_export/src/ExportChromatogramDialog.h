#ifndef _U2_EXPORT_CHROMATOGRAM_DIALOG_H_
#define _U2_EXPORT_CHROMATOGRAM_DIALOG_H_

#include <U2Core/global.h>
#include <U2Core/GUrl.h>

#include <QtGui/QDialog>
#include <ui/ui_ExportChromatogramDialog.h>

namespace U2 {

class SaveDocumentGroupController;

class ExportChromatogramDialog : public QDialog, Ui_ExportChromatogramDialog {
    Q_OBJECT
public:
    ExportChromatogramDialog(QWidget* p, const GUrl& fileName);

    virtual void accept();


public:    
    QString url;
    DocumentFormatId format;
    bool    addToProjectFlag;
    bool reversed;
    bool complemented;
    
private slots:
    void sl_onBrowseClicked();

private:    
    SaveDocumentGroupController* saveController;

};

}//namespace

#endif
