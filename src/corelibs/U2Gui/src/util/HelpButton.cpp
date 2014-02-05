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

#include "HelpButton.h"

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QPushButton>
#include <QUrl>

namespace U2{

HelpButton::HelpButton(QWidget *target ,QDialogButtonBox *b, QString lnk):QObject(target), link(lnk){
    QPushButton *hb = new QPushButton(tr("Help"));
    connect(hb, SIGNAL(clicked()), SLOT(sl_buttonClicked()));
    b->addButton(hb, QDialogButtonBox::HelpRole);
}

void HelpButton::sl_buttonClicked(){
    QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode));
}

}
