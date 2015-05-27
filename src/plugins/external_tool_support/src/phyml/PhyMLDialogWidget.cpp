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

#include <QMessageBox>

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/QObjectScopedPointer.h>
#include <U2Gui/U2FileDialog.h>

#include "ExternalToolSupportSettings.h"
#include "ExternalToolSupportSettingsController.h"
#include "PhyMLDialogWidget.h"
#include "PhyMLSupport.h"

namespace U2 {

const QString PhyMlSettingsPreffixes::ModelType(CreatePhyTreeWidget::settingsPath + "/phyml_model_t");
const QString PhyMlSettingsPreffixes::OptimiseEquilibriumFreq(CreatePhyTreeWidget::settingsPath + "/phyml_eq_freq_flag");

const QString PhyMlSettingsPreffixes::EstimateTtRatio(CreatePhyTreeWidget::settingsPath + "/phyml_est_trans_ratio");
const QString PhyMlSettingsPreffixes::TtRatio(CreatePhyTreeWidget::settingsPath + "/phyml_trans_ratio");

const QString PhyMlSettingsPreffixes::EstimateSitesProportion(CreatePhyTreeWidget::settingsPath + "/phyml_est_sites_prop");
const QString PhyMlSettingsPreffixes::InvariableSitesProportion(CreatePhyTreeWidget::settingsPath + "/phyml_sites_proportion");

const QString PhyMlSettingsPreffixes::EstimateGammaFactor(CreatePhyTreeWidget::settingsPath + "/phyml_est_gamma");
const QString PhyMlSettingsPreffixes::GammaFactor(CreatePhyTreeWidget::settingsPath + "/phyml_gamma");

const QString PhyMlSettingsPreffixes::UseBootstrap(CreatePhyTreeWidget::settingsPath + "/phyml_use_boot");
const QString PhyMlSettingsPreffixes::BootstrapReplicatesNumber(CreatePhyTreeWidget::settingsPath + "/phyml_boot_repl");

const QString PhyMlSettingsPreffixes::UseFastMethod(CreatePhyTreeWidget::settingsPath + "/phyml_use_fast");
const QString PhyMlSettingsPreffixes::FastMethodIndex(CreatePhyTreeWidget::settingsPath + "/phyml_fast_method");

const QString PhyMlSettingsPreffixes::SubRatesNumber(CreatePhyTreeWidget::settingsPath + "/phyml_sub_rates");
const QString PhyMlSettingsPreffixes::OptimiseTopology(CreatePhyTreeWidget::settingsPath + "/phyml_opt_topol");
const QString PhyMlSettingsPreffixes::OptimiseBranchLenghs(CreatePhyTreeWidget::settingsPath + "/phyml_opt_lengths");

const QString PhyMlSettingsPreffixes::TreeImprovementType(CreatePhyTreeWidget::settingsPath + "/phyml_tree_impr");
const QString PhyMlSettingsPreffixes::TreeSearchingType(CreatePhyTreeWidget::settingsPath + "/phyml_search");
const QString PhyMlSettingsPreffixes::UserTreePath(CreatePhyTreeWidget::settingsPath + "/user_tree");

PhyMlWidget::PhyMlWidget(QWidget* parent, const MAlignment& ma)
    : CreatePhyTreeWidget(parent)
{
    setupUi(this);
    fillComboBoxes(ma);
    createWidgetsControllers();

    widgetControllers.getDataFromSettings();
    sl_checkSubModelType(subModelCombo->currentText());

    connect(subModelCombo, SIGNAL(currentIndexChanged(const QString&)), SLOT(sl_checkSubModelType(const QString&)));
    connect(treeTypesCombo, SIGNAL(currentIndexChanged(int)), SLOT(sl_checkUserTreeType(int )));
    connect(treeImprovementsCombo, SIGNAL(currentIndexChanged(int)), SLOT(sl_checkTreeImprovement(int )));
    connect(inputFilePathButton, SIGNAL(clicked ()), SLOT(sl_inputPathButtonClicked()));
}

void PhyMlWidget::fillComboBoxes(const MAlignment& ma) {
    DNAAlphabetType alphabetType = ma.getAlphabet()->getType();
    if ((alphabetType == DNAAlphabet_RAW) || (alphabetType == DNAAlphabet_NUCL)){
        isAminoAcid = false;
        subModelCombo->addItems(PhyMLModelTypes::getDnaModelTypes());
    } else {
        isAminoAcid = true;
        subModelCombo->addItems(PhyMLModelTypes::getAminoAcidModelTypes());
        tranCheckBox->setEnabled(false);
    }
    fastMethodCombo->addItems(PhyMLRatioTestsTypes::getRatioTestsTypes());
    treeTypesCombo->addItems(TreeSearchingParams::getInputTreeTypes());
    treeImprovementsCombo->addItems(TreeSearchingParams::getTreeImprovementTypes());
}

void PhyMlWidget::createWidgetsControllers() {
    //Substitutional model
    widgetControllers.addWidgetController(subModelCombo, PhyMlSettingsPreffixes::ModelType, "-m");

    //Number of substitution rate categories
    widgetControllers.addWidgetController(substitutionSpinBox, PhyMlSettingsPreffixes::SubRatesNumber, "-c");

    //Transition / transversion ratio
    InputWidgetController* ttRatioEstimationController = widgetControllers.addWidgetController(tranCheckBox, PhyMlSettingsPreffixes::EstimateTtRatio, "");
    InputWidgetController* ttRatioController = widgetControllers.addWidgetController(tranSpinBox, PhyMlSettingsPreffixes::TtRatio, "-t");
    ttRatioEstimationController->addDependentParameter(ParameterDependence(ttRatioController, true));

    //Proportion of invariable sites
    InputWidgetController* sitesEstimationController = widgetControllers.addWidgetController(sitesCheckBox, PhyMlSettingsPreffixes::EstimateSitesProportion, "");
    InputWidgetController* sitesPropController = widgetControllers.addWidgetController(sitesSpinBox, PhyMlSettingsPreffixes::InvariableSitesProportion, "-t");
    sitesEstimationController->addDependentParameter(ParameterDependence(sitesPropController, true));

    //Gamma shape parameter
    InputWidgetController* gammaEstimationController = widgetControllers.addWidgetController(gammaCheckBox, PhyMlSettingsPreffixes::EstimateGammaFactor, "");
    InputWidgetController* gammaController = widgetControllers.addWidgetController(gammaSpinBox, PhyMlSettingsPreffixes::GammaFactor, "-a");
    gammaEstimationController->addDependentParameter(ParameterDependence(gammaController, true));

    //Bootstrap replicates number
    InputWidgetController* bootstrapCheckBoxController = widgetControllers.addWidgetController(bootstrapCheckBox, PhyMlSettingsPreffixes::UseBootstrap, "");
    InputWidgetController* bootstrapController = widgetControllers.addWidgetController(bootstrapSpinBox, PhyMlSettingsPreffixes::BootstrapReplicatesNumber, "-b");
    bootstrapCheckBoxController->addDependentParameter(ParameterDependence(bootstrapController , true));

    //Optimisation options
    widgetControllers.addWidgetController(optBranchCheckbox, PhyMlSettingsPreffixes::OptimiseBranchLenghs, "");
    widgetControllers.addWidgetController(optTopologyCheckbox, PhyMlSettingsPreffixes::OptimiseTopology, "");

    //Fast methods
    InputWidgetController* fastMethodCheckBoxController = widgetControllers.addWidgetController(fastMethodCheckbox, PhyMlSettingsPreffixes::UseFastMethod, "");
    QStringList cmdLineValues;
    cmdLineValues << "-1" << "-2" << "-4";
    InputWidgetController* fastMethodController = widgetControllers.addWidgetController(fastMethodCombo, PhyMlSettingsPreffixes::FastMethodIndex, "-b", cmdLineValues);
    fastMethodCheckBoxController->addDependentParameter(ParameterDependence(fastMethodController, true));

    //Tree improvements
    QStringList treeImprovements;
    treeImprovements << "NNI" << "SPR" << "BEST";
    widgetControllers.addWidgetController(treeImprovementsCombo, PhyMlSettingsPreffixes::TreeImprovementType, "-s", treeImprovements);

    //Equilibrium frequencies
    widgetControllers.addWidgetController(freqOptimRadio, PhyMlSettingsPreffixes::OptimiseEquilibriumFreq, "");

    //Generated tree numbers
    InputWidgetController* treeNumbersCheckBoxController = widgetControllers.addWidgetController(treeNumbersCheckbox , PhyMlSettingsPreffixes::UseBootstrap, "");
    InputWidgetController* treeNumbersController = widgetControllers.addWidgetController(treeNumbersSpinBox, PhyMlSettingsPreffixes::BootstrapReplicatesNumber, "-b");
    treeNumbersCheckBoxController->addDependentParameter(ParameterDependence(treeNumbersController, true));

    //Tree searching
    widgetControllers.addWidgetController(treeTypesCombo, PhyMlSettingsPreffixes::TreeSearchingType, "");
    //User tree
    widgetControllers.addWidgetController(inputFileLineEdit, PhyMlSettingsPreffixes::UserTreePath, "");
}

void PhyMlWidget::sl_checkUserTreeType(int newIndex) {
    inputTreeGroupBox->setEnabled(newIndex == 1);
}

void PhyMlWidget::sl_checkTreeImprovement(int newIndex) {
    bool isNNI = (newIndex == 0);
    treeNumbersCheckbox->setEnabled(!isNNI);
    if(isNNI) {
        treeNumbersSpinBox->setEnabled(false);
    }
}

void PhyMlWidget::sl_inputPathButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = U2FileDialog::getOpenFileName(this, tr("Open an alignment file"), lod.dir,
        DialogUtils::prepareDocumentsFileFilter(BaseDocumentFormats::NEWICK, false));
    if (lod.url.isEmpty()) {
        return;
    }
    inputFileLineEdit->setText(lod.url);
}

void PhyMlWidget::sl_checkSubModelType(const QString& newModel){
    if(isAminoAcid) {
        tranCheckBox->setEnabled(false);
        return;
    }

    const QStringList& allDnaModels = PhyMLModelTypes::getDnaModelTypes();
    int modelIndex = allDnaModels.indexOf(newModel);
    SAFE_POINT(modelIndex >= 0, QString("'%1' is incorrect substitution model for dna sequence").arg(newModel),);

    SubstModelTrRatioType ttRatioType = PhyMLModelTypes::getTtRatioType(newModel);
    if(ttRatioType == ANY_TT_RATIO) {
        tranCheckBox->setEnabled(true);
    }
    else {
        tranCheckBox->setChecked(ttRatioType == ONLY_FIXED_TT_RATIO);
        tranCheckBox->setEnabled(false);
    }
}

void PhyMlWidget::fillSettings(CreatePhyTreeSettings& settings){
    settings.extToolArguments = generatePhyMlSettingsScript();
    settings.bootstrap = bootstrapCheckBox->isChecked();
}

void PhyMlWidget::storeSettings(){
    widgetControllers.storeSettings();
}

void PhyMlWidget::restoreDefault(){
    widgetControllers.restoreDefault();
    sl_checkSubModelType(subModelCombo->currentText());
}

bool PhyMlWidget::checkSettings(QString& , const CreatePhyTreeSettings& ){
    //Check that PhyMl and tempory directory path defined
    ExternalToolRegistry* reg = AppContext::getExternalToolRegistry();
    ExternalTool* phyml= reg->getByName(PhyMLSupport::PhyMlRegistryId);
    SAFE_POINT(NULL != phyml, "External tool PHyML is not registered", false);
    const QString& path = phyml->getPath();
    const QString& name = phyml->getName();
    if (path.isEmpty()){
        QObjectScopedPointer<QMessageBox> msgBox = new QMessageBox;
        msgBox->setWindowTitle(name);
        msgBox->setText(tr("Path for %1 tool not selected.").arg(name));
        msgBox->setInformativeText(tr("Do you want to select it now?"));
        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox->setDefaultButton(QMessageBox::Yes);
        int ret = msgBox->exec();
        CHECK(!msgBox.isNull(), false);

        switch (ret) {
           case QMessageBox::Yes:
               AppContext::getAppSettingsGUI()->showSettingsDialog(ExternalToolSupportSettingsPageId);
               break;
           case QMessageBox::No:
               return false;
               break;
           default:
               SAFE_POINT(false, "Incorrect state of the message box", false);
               break;
        }
    }
    if (path.isEmpty()){
        return false;
    }
    U2OpStatus2Log os(LogLevel_DETAILS);
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, false);

    return true;
}

QStringList PhyMlWidget::generatePhyMlSettingsScript(){
    QStringList script;
    if(isAminoAcid) {
        script << "-d";
        script << "aa";
    }

    widgetControllers.addParametersToCmdLine(script);

    if(1 == treeTypesCombo->currentIndex()) {
        script << "-u";
        script << inputFileLineEdit->text();
    }

    QString optimisationOptions;
    if(optTopologyCheckbox->isChecked()) {
        optimisationOptions = "t";
    }
    if(optBranchCheckbox->isChecked()) {
        optimisationOptions += "l";
    }
    if(!optimisationOptions.isEmpty()) {
        script << "-o";
        script << optimisationOptions;
    }

    return script;
}

}//namespace
