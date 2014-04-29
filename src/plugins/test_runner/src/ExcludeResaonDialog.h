#ifndef EXCLUDERESAONDIALOG_H
#define EXCLUDERESAONDIALOG_H

#include <QDialog>

namespace Ui {
class ExcludeResaonDialog;
}
namespace U2{

class ExcludeResaonDialog : public QDialog
{
    Q_OBJECT
    
public:
    ExcludeResaonDialog(QWidget *parent = 0);
    ~ExcludeResaonDialog();
    QString getReason();
private:
    Ui::ExcludeResaonDialog *ui;
};

}
#endif // EXCLUDERESAONDIALOG_H
