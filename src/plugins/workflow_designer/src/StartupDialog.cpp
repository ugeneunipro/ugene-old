/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "StartupDialog.h"

#include <QtCore/QDir>

#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>


namespace U2 {

StartupDialog::StartupDialog(QWidget *parent)
: QDialog(parent)
{
    setupUi(this);
    outDirWidget = new OutputDirectoryWidget(this);
    QVBoxLayout *l = new QVBoxLayout(box);
    l->setMargin(3);
    l->addWidget(outDirWidget);
    buttons->addButton(tr("Don't use the directory"), QDialogButtonBox::RejectRole);
    connect(buttons, SIGNAL(accepted()), this, SLOT(sl_accepted()));

#ifdef Q_OS_WIN
    QFont f( "Arial", 8, QFont::Bold);
    label->setFont(f);
#endif

}

void StartupDialog::sl_accepted(){
    QDir dir(outDirWidget->getChoosenDir());
    QFile file(dir.filePath("ex1.txt"));
    if (!file.open(QIODevice::WriteOnly)){
        QMessageBox::critical(this, this->windowTitle(), tr("You have no write access to the directory. Please choose another one."));
        return;
    }
    file.close();
    file.remove();
    QDialog::accept();
}

} // U2
