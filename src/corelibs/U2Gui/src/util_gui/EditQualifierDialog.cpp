#include "EditQualifierDialog.h"
#include <ui/ui_EditQualifierDialog.h>

#include <QtGui/QMessageBox>
#include <QtGui/QKeyEvent>

namespace U2 {

EditQualifierDialog::EditQualifierDialog(QWidget* p, const U2Qualifier& q, bool ro, bool existingQualifier)
: QDialog(p)
{
    ui = new Ui_EditQualifierDialog;
    ui->setupUi(this);
    
    if( !existingQualifier ) { // adding new qualifier
        setWindowTitle("Add new qualifier");
    }
    
    ui->nameEdit->setReadOnly(ro);
    ui->valueEdit->setReadOnly(ro);

    ui->nameEdit->setText(q.name);
    ui->valueEdit->setText(q.value);

    ui->valueEdit->installEventFilter(this);
}

bool EditQualifierDialog::eventFilter(QObject *obj, QEvent *e) {
    Q_UNUSED(obj);
    QEvent::Type t = e->type();
    if (t == QEvent::KeyPress) {
        QKeyEvent* ke = (QKeyEvent*)e;
        int key = ke->key();
        if (key == Qt::Key_Tab) {
            ui->nameEdit->setFocus();
            return true;
        } 
        if (key == Qt::Key_Enter || key == Qt::Key_Return) {
            accept();
            return true;
        }
    }
    return false;
}

static QString simplify(const QString& s) {
    QString res = s;
    res = res.replace("\t", "    ");
    res = res.replace("\r", "");
    res = res.replace("\n", " ");
    res = res.trimmed();
    return res;
}

void EditQualifierDialog::accept() {
    if (ui->nameEdit->isReadOnly()) {
        QDialog::accept();
        return;
    }
    QString name = simplify(ui->nameEdit->text());
    QString val = simplify(ui->valueEdit->toPlainText());
    if (!Annotation::isValidQualifierName(name)) {
        QMessageBox::critical(this, tr("Error!"), tr("Illegal qualifier name"));
        return;
    }
    if (!Annotation::isValidQualifierValue(val)) {
        QMessageBox::critical(this, tr("Error!"), tr("Illegal qualifier value"));
        return;
    }
    q = U2Qualifier(name, val);
    QDialog::accept();
}

EditQualifierDialog::~EditQualifierDialog()
{
    delete ui;
}
}//namespace
