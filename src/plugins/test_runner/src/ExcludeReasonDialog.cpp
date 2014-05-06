#include "ExcludeReasonDialog.h"
#include "ui_ExcludeResaonDialog.h"

namespace U2{

ExcludeReasonDialog::ExcludeReasonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExcludeResaonDialog)
{
    ui->setupUi(this);
}

ExcludeReasonDialog::~ExcludeReasonDialog()
{
    delete ui;
}

QString ExcludeReasonDialog::getReason(){
    return ui->lineEdit->text();
}

}
