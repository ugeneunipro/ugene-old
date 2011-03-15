/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _SEQ_BOOT_MODEL_WIDGET_H_
#define _SEQ_BOOT_MODEL_WIDGET_H_

#include <U2View/CreatePhyTreeWidget.h>
#include "ui/ui_SeqBootModel.h"

#include <U2Core/MAlignment.h>


namespace U2{

class SeqBootModelWidget : public CreatePhyTreeWidget, Ui_SeqBootModel {
    Q_OBJECT

public:
    SeqBootModelWidget(QWidget* parent, const MAlignment& ma);
    virtual void fillSettings(CreatePhyTreeSettings& settings);
private slots:
    void sl_onCheckBox();


};

}

#endif