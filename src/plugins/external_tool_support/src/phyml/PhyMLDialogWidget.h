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

#ifndef _U2_PHYML_DIALOGWIDGET_H
#define _U2_PHYML_DIALOGWIDGET_H

#include "ui/ui_PhyMLDialog.h"

#include "PhyMLSupport.h"

#include <U2Gui/InputWidgetsControllers.h>
#include <U2Core/MAlignment.h>
#include <U2Core/AppContext.h>
#include <U2View/CreatePhyTreeWidget.h>

namespace U2 {

class PhyMlSettingsPreffixes {
public:
    static const QString ModelType;
    static const QString OptimiseEquilibriumFreq;

    static const QString EstimateTtRatio;
    static const QString TtRatio;

    static const QString EstimateSitesProportion;
    static const QString InvariableSitesProportion;

    static const QString EstimateGammaFactor;
    static const QString GammaFactor;

    static const QString UseBootstrap;
    static const QString BootstrapReplicatesNumber;

    static const QString UseFastMethod;
    static const QString FastMethodIndex;

    static const QString SubRatesNumber;
    static const QString OptimiseTopology;
    static const QString OptimiseBranchLenghs;

    static const QString TreeImprovementType;
    static const QString TreeSearchingType;
    static const QString UserTreePath;
};

class PhyMlWidget : public CreatePhyTreeWidget, Ui_PhyMLDialog{
   Q_OBJECT

public:
    PhyMlWidget(QWidget* parent, const MAlignment& ma);

    void fillSettings(CreatePhyTreeSettings& settings); 
    void storeSettings();
    void restoreDefault();
    bool checkSettings(QString& msg, const CreatePhyTreeSettings& settings);

private:
    void createWidgetsControllers();
    void fillComboBoxes(const MAlignment& ma);

    QStringList generatePhyMlSettingsScript();

    bool isAminoAcid; //is the msa has the amino acid alphabet

    WidgetControllersContainer widgetControllers; 
private slots:
    void sl_checkUserTreeType(int newIndex);
    void sl_checkTreeImprovement(int newIndex);
    void sl_checkSubModelType(const QString& newModel);
    void sl_inputPathButtonClicked();
};

}//namespace

#endif // _U2_PHYML_DIALOGWIDGET_H
