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

#include <QVBoxLayout>

#include <U2Designer/OutputDirectoryWidget.h>

#include "StartupDialog.h"

namespace U2 {

StartupDialog::StartupDialog(QWidget *parent)
: QDialog(parent)
{
    setupUi(this);
    QVBoxLayout *l = new QVBoxLayout(box);
    l->setMargin(3);
    l->addWidget(new OutputDirectoryWidget(this));
    buttons->addButton(tr("Don't use the directory"), QDialogButtonBox::RejectRole);

#ifdef Q_OS_WIN
    QFont f( "Arial", 8, QFont::Bold);
    label->setFont(f);
#endif

}

} // U2
