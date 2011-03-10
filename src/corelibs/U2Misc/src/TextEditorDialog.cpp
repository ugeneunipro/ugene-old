#include "TextEditorDialog.h"
#include <ui/ui_TextEditorDialog.h>

#include <QtGui/QKeyEvent>

namespace U2 {

TextEditorDialog::TextEditorDialog(QWidget* parent, const QString& title, const QString& label, const QString& text, bool acceptOnEnter) 
: QDialog(parent)
{
    ui = new Ui_TextEditorDialog();
    ui->setupUi(this);
    setWindowTitle(title);
    ui->valueLabel->setText(label);
    ui->textEdit->setPlainText(text);
    if (acceptOnEnter) {
        ui->textEdit->installEventFilter(this);
    }
}

bool TextEditorDialog::eventFilter(QObject *o, QEvent *e) {
    if (o == ui->textEdit) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent* ke = (QKeyEvent*)e;
            if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
                accept();
                return true;
            }
        }
    }
    return false;
}

QString TextEditorDialog::getText()
{
    return ui->textEdit->toPlainText();
}

TextEditorDialog::~TextEditorDialog()
{
    delete ui;
}
} //namespace
