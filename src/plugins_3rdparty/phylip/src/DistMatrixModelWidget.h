/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _DIST_MATRIX_MODEL_WIDGET_H_
#define _DIST_MATRIX_MODEL_WIDGET_H_

#include <U2View/CreatePhyTreeWidget.h>
#include "ui/ui_DistMatrixModel.h"

#include <U2Core/MAlignment.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>


namespace U2{

class DistMatrixModelWidget : public CreatePhyTreeWidget, Ui_DistMatrixModel {
    Q_OBJECT

public:
    DistMatrixModelWidget(QWidget* parent, const MAlignment& ma);
    virtual void fillSettings(CreatePhyTreeSettings& settings);
    virtual void storeSettings();
    virtual void restoreDefault();
    virtual bool checkMemoryEstimation(QString& msg, const MAlignment& msa, const CreatePhyTreeSettings& settings);
private slots:
    void sl_onModelChanged(const QString& modelName);


};

}

#endif