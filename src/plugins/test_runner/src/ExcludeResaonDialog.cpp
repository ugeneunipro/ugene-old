#include "ExcludeResaonDialog.h"
#include "ui_ExcludeResaonDialog.h"

namespace U2{

ExcludeResaonDialog::ExcludeResaonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExcludeResaonDialog)
{
    ui->setupUi(this);
}

ExcludeResaonDialog::~ExcludeResaonDialog()
{
    delete ui;
}

QString ExcludeResaonDialog::getReason(){
    return ui->lineEdit->text();
}

}
