#ifndef EXCLUDERESAONDIALOG_H
#define EXCLUDERESAONDIALOG_H

#include <QDialog>

class Ui_ExcludeReasonDialog;

namespace U2 {

class ExcludeReasonDialog : public QDialog {
    Q_OBJECT

public:
    ExcludeReasonDialog(QWidget *parent = 0);
    ~ExcludeReasonDialog();
    QString getReason();
private:
    Ui_ExcludeReasonDialog *ui;
};

}
#endif // EXCLUDERESAONDIALOG_H
