#ifndef _U2_TCOFFEE_SUPPORT_RUN_DIALOG_H
#define _U2_TCOFFEE_SUPPORT_RUN_DIALOG_H

#include <QtGui/QDialog>
#include <U2Misc/DialogUtils.h>
#include <ui/ui_TCoffeeSupportRunDialog.h>
#include "TCoffeeSupportTask.h"

namespace U2 {

class TCoffeeSupportRunDialog : public QDialog, public Ui_TCoffeeSupportRunDialog {
    Q_OBJECT
public:
    TCoffeeSupportRunDialog(TCoffeeSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_align();
private:
    TCoffeeSupportTaskSettings&        settings;
};

class TCoffeeWithExtFileSpecifySupportRunDialog : public QDialog, public Ui_TCoffeeSupportRunDialog{
    Q_OBJECT
public:
    TCoffeeWithExtFileSpecifySupportRunDialog(TCoffeeSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_align();
    void sl_inputFileLineEditChanged(const QString& str);
private:
    TCoffeeSupportTaskSettings&     settings;
    FileLineEdit*                   inputFileLineEdit;
};

}//namespace
#endif // _U2_TCOFFEE_SUPPORT_RUN_DIALOG_H
