/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "CreatePhyTreeWidget.h"

namespace U2 {

const QString CreatePhyTreeWidget::settingsPath = "phytree_dialog_settings";

CreatePhyTreeWidget::CreatePhyTreeWidget(QWidget *parent) :
    QWidget(parent)
{

}

bool CreatePhyTreeWidget::checkSettings(QString &, const CreatePhyTreeSettings &) {
    return true;
}

bool CreatePhyTreeWidget::checkMemoryEstimation(QString &, const MAlignment &, const CreatePhyTreeSettings &) {
    return true;
}

bool CreatePhyTreeWidget::insertOutputOptionWidget(PhyTreeDisplayOptionsWidget *outputOptionsWidget) {
    return false;
}

}
