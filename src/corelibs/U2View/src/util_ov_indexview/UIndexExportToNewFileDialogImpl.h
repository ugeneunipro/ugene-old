
#ifndef _U2_UINDEX_EXPORT_TO_NEW_DIALOG_IMPL_H_
#define _U2_UINDEX_EXPORT_TO_NEW_DIALOG_IMPL_H_

#include <ui/ui_UIndexExportToNewFileDialog.h>

namespace U2 {

struct UIndexExportToNewFileDialogModel {
    QString exportFilename;
    bool    gzipExported;
};

class UIndexExportToNewFileDialogImpl : public QDialog, public Ui::UIndexExportToNewFileDialog {
    Q_OBJECT
public:
    static const QString SETTINGS_LAST_DIRECTORY;
    
    UIndexExportToNewFileDialogImpl();
    
    UIndexExportToNewFileDialogModel getModel() const;
    
    virtual void accept();

private slots:
    void sl_openExportFile();
    
private:
    UIndexExportToNewFileDialogModel model;
    
}; // UIndexExportToNewFileDialogImpl

} // U2

#endif // _U2_UINDEX_EXPORT_TO_NEW_DIALOG_IMPL_H_
