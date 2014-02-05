/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "TextEditorDialog.h"
#include "ui/ui_TextEditorDialog.h"

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
