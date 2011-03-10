#ifndef _U2_FORMATDB_SUPPORT_RUN_DIALOG_H
#define _U2_FORMATDB_SUPPORT_RUN_DIALOG_H

#include <QDialog>
#include <U2Misc/DialogUtils.h>
#include <ui/ui_FormatDBSupportRunDialog.h>
#include "FormatDBSupportTask.h"

namespace U2 {

class FormatDBSupportRunDialog : public QDialog, public Ui_FormatDBSupportRunDialog {
    Q_OBJECT
public:
    FormatDBSupportRunDialog(FormatDBSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_formatDB();
    void sl_lineEditChanged();

    void sl_onBrowseInputFiles();
    void sl_onBrowseInputDir();
    void sl_onBrowseDatabasePath();
private:
    FormatDBSupportTaskSettings&    settings;
};

}//namespace
#endif // _U2_FORMATDB_SUPPORT_RUN_DIALOG_H
