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

#include "HBar.h"

namespace U2 {

void HBar::setButtonTabOrderList(QList<QString> * buttonNamesInNeededOrder) {
    assert(NULL != buttonNamesInNeededOrder);
    buttonTabOrderList = buttonNamesInNeededOrder;
}

void HBar::setVisible(bool visible)
{
    QWidget::setVisible(visible);

    if(!tabOrdered && NULL != buttonTabOrderList) {
        setButtonsTabOrder();
        tabOrdered = true;
    }
}

void HBar::setButtonsTabOrder() const {
    assert(!buttonTabOrderList->isEmpty());

    QList<QObject *> barElements = children();
    QWidget * prevButton = NULL;
    QWidget * curButton = NULL;

    for(QList<QString>::const_iterator it = buttonTabOrderList->constBegin(); it != buttonTabOrderList->constEnd(); it++) {
        foreach(QObject * element, barElements) {
            if(element->objectName().contains(*it)) {
                if(NULL != prevButton) {
                    curButton = qobject_cast<QWidget *>(element);
                } else {
                    prevButton = qobject_cast<QWidget *>(element);
                }
                if(NULL == curButton || NULL == prevButton) {
                    break;
                }
                
                QWidget::setTabOrder(prevButton, curButton);
                prevButton = curButton;

                break;
            }
        }
        assert(NULL != prevButton);
    }
}

} //namespace
