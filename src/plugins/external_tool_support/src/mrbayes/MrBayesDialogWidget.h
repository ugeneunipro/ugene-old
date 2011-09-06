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

#ifndef _U2_MRBAYES_DIALOGWIDGET_H
#define _U2_MRBAYES_DIALOGWIDGET_H

#include "ui/ui_MrBayesDialog.h"

#include <U2Core/MAlignment.h>
#include <U2Core/AppContext.h>
#include <U2View/CreatePhyTreeWidget.h>

namespace U2 {

class MrBayesWidget : public CreatePhyTreeWidget, Ui_MrBayesDialog{
   Q_OBJECT

public:
    MrBayesWidget(QWidget* parent, const MAlignment& ma);
    virtual void fillSettings(CreatePhyTreeSettings& settings); 
    virtual void storeSettings();
    virtual void restoreDefault();
    int getRandomSeed();
    virtual bool checkSettings(QString& msg, const CreatePhyTreeSettings& settings);

private:
    void setComboText(QComboBox* combo, const QString& text);
    QString generateMrBayesSettingsScript();
    bool isAA; //is the msa has the amino acid alphabet

private slots:
     void sl_onRateChanged(const QString& modelName);
};

}//namespace

#endif // _U2_MRBAYES_DIALOGWIDGET_H
