#ifndef EXCLUDERESAONDIALOG_H
#define EXCLUDERESAONDIALOG_H

#include <QDialog>

namespace Ui {
class ExcludeReasonDialog;
}
namespace U2{

class ExcludeReasonDialog : public QDialog
{
    Q_OBJECT
    
public:
    ExcludeReasonDialog(QWidget *parent = 0);
    ~ExcludeReasonDialog();
    QString getReason();
private:
    Ui::ExcludeReasonDialog *ui;
};

}
#endif // EXCLUDERESAONDIALOG_H
