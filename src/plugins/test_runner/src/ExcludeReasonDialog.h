#ifndef EXCLUDERESAONDIALOG_H
#define EXCLUDERESAONDIALOG_H

#include <QDialog>

namespace Ui {
class ExcludeResaonDialog;
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
    Ui::ExcludeResaonDialog *ui;
};

}
#endif // EXCLUDERESAONDIALOG_H
