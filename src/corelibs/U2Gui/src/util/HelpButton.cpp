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

#include "HelpButton.h"

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QPushButton>
#include <QUrl>

namespace U2{

HelpButton::HelpButton(QWidget *target, QString lnk, QString layoutName):QObject(target), link(lnk){
    QList<QHBoxLayout*> layouts = target->findChildren<QHBoxLayout*>();
    QHBoxLayout *bl = NULL;
    foreach(QHBoxLayout *layout, layouts){
        if(layout->objectName() == layoutName){
            bl = layout;
            break;
        }
    }
    if(bl != NULL){
        QPushButton *hb = new QPushButton(tr("Help"));
        bl->insertWidget(0, hb);
        connect(hb, SIGNAL(clicked()), SLOT(sl_buttonClicked()));
    }    
}

void HelpButton::sl_buttonClicked(){
    QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode));
}

}
