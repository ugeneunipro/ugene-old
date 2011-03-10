#ifndef _U2_CLUSTALW_SUPPORT_RUN_DIALOG_H
#define _U2_CLUSTALW_SUPPORT_RUN_DIALOG_H

#include <QDialog>
#include <U2Misc/DialogUtils.h>
#include <ui/ui_ClustalWSupportRunDialog.h>
#include "ClustalWSupportTask.h"

namespace U2 {

class ClustalWSupportRunDialog : public QDialog, public Ui_ClustalWSupportRunDialog {
    Q_OBJECT
public:
    ClustalWSupportRunDialog(const MAlignment& ma, ClustalWSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_align();
    void sl_iterationTypeEnabled(bool checked);
private:
    MAlignment                      ma;
    ClustalWSupportTaskSettings&    settings;

};

class ClustalWWithExtFileSpecifySupportRunDialog : public QDialog, public Ui_ClustalWSupportRunDialog {
    Q_OBJECT
public:
    ClustalWWithExtFileSpecifySupportRunDialog(ClustalWSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_align();
    void sl_iterationTypeEnabled(bool checked);
    void sl_inputFileLineEditChanged(const QString& str);
private:
    ClustalWSupportTaskSettings&    settings;
    FileLineEdit*                   inputFileLineEdit;
};

}//namespace
#endif // _U2_CLUSTALW_SUPPORT_RUN_DIALOG_H
