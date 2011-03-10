#ifndef _U2_AUTHENTIFICATION_DIALOG_H_
#define _U2_AUTHENTIFICATION_DIALOG_H_

#include <U2Core/global.h>
#include <QtGui/QDialog>

class Ui_AuthentificationDialog;

namespace U2 {

class U2MISC_EXPORT AuthDialog : public QDialog {
    Q_OBJECT
public:
    AuthDialog(QWidget* parent);
    ~AuthDialog();
    QString getUserName() const;
    QString getPasswd() const;
    bool    rememberAuthData() const;

private slots:
    void sl_onOkButtonClicked();

private:
    Ui_AuthentificationDialog* ui;
};

} // namespace

#endif
