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

#include "TreeOptionsWidget.h"

#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Core/AppContext.h>
#include <U2Core/L10n.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2SafePoints.h>

#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include "ov_msa/MSAEditorDataList.h"
#include "ov_phyltree/TreeViewer.h"
#include "ov_msa/PhyTrees/MSAEditorMultiTreeViewer.h"
#include "phyltree/TreeSettingsDialog.h"

namespace U2 {

static const int ITEMS_SPACING = 10;
static const int TITLE_SPACING = 5;

const static QString SHOW_FONT_OPTIONS_LINK("show_font_options_link");
const static QString SHOW_PEN_OPTIONS_LINK("show_pen_options_link");

static inline QVBoxLayout * initLayout(QWidget * w) {
    QVBoxLayout * layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignTop);

    w->setLayout(layout);
    return layout;
}

const QString TreeOptionsWidget::COLOR_BOX_STYLE = "QPushButton {"
                                                        "border: none;"
                                                        "min-height: 20px;"
                                                        "background-color : %1;}";

TreeOptionsWidget::TreeOptionsWidget(MSAEditor* m, const TreeOpWidgetViewSettings& _viewSettings)
    : msa(m), treeViewer(NULL), viewSettings(_viewSettings), settings(NULL), showFontSettings(false), showPenSettings(false)
{
    SAFE_POINT(NULL != msa, QString("Invalid parameter were passed into constructor TreeOptionsWidget"), );

    contentWidget = new QWidget();
    setupUi(contentWidget);
    treeSettings = getTreeViewer()->getTreeSettings();
    createGroups();
}

TreeOptionsWidget::TreeOptionsWidget(TreeViewer* tree, const TreeOpWidgetViewSettings& _viewSettings)
    : msa(NULL), treeViewer(tree->getTreeViewerUI()), viewSettings(_viewSettings), settings(NULL), showFontSettings(false), showPenSettings(false)
{
    SAFE_POINT(NULL != treeViewer, QString("Invalid parameter were passed into constructor TreeOptionsWidget"), );

    contentWidget = new QWidget();
    setupUi(contentWidget);
    treeSettings = treeViewer->getTreeSettings();
    createGroups();
}

TreeOptionsWidget::~TreeOptionsWidget() 
{
    emit saveViewSettings(getViewSettings());
    delete contentWidget;
}

const TreeOpWidgetViewSettings& TreeOptionsWidget::getViewSettings() {
    viewSettings.openBranchesGroup = branchesOpGroup->isSubgroupOpened();
    viewSettings.openGeneralGroup = generalOpGroup->isSubgroupOpened();
    viewSettings.openLabelsGroup = labelsOpGroup->isSubgroupOpened();
    viewSettings.showFontSettings = showFontSettings;
    viewSettings.showPenSettings = showPenSettings;
    return viewSettings;
}

void TreeOptionsWidget::createGroups() {
    QVBoxLayout* mainLayout = initLayout(this);
    mainLayout->setSpacing(0);

    generalOpGroup = new ShowHideSubgroupWidget("TREE_GENERAL_OP", tr("General"), treeLayoutWidget, viewSettings.openGeneralGroup);
    mainLayout->addWidget(generalOpGroup);
    labelsOpGroup = new ShowHideSubgroupWidget("TREE_LABELS_OP", tr("Labels"), labelsGroup, viewSettings.openLabelsGroup);
    mainLayout->addWidget(labelsOpGroup);
    branchesOpGroup = new ShowHideSubgroupWidget("TREE_BRANCHES_OP", tr("Branches"), branchesGroup, viewSettings.openBranchesGroup);
    mainLayout->addWidget(branchesOpGroup);
    
    updateAllWidgets();
    connectSlots();
}

void TreeOptionsWidget::updateAllWidgets()
{
    showFontSettings = viewSettings.showFontSettings;
    showPenSettings = viewSettings.showPenSettings;
    createGeneralSettingsWidgets();
    updateLabelsSettingsWidgets();
    updateFormatSettings();
    updateBranchSettings();

    updateShowFontOpLabel("Show font settings");
    updateShowPenOpLabel("Show pen settings");
    fontSettingsWidget->setVisible(showFontSettings);
    penGroup->setVisible(showPenSettings);
}

void TreeOptionsWidget::connectSlots()
{
    // Show more options labels
    connect(lblPenSettings, SIGNAL(linkActivated(const QString&)), SLOT(sl_onLblLinkActivated(const QString&)));
    connect(lblFontSettings, SIGNAL(linkActivated(const QString&)), SLOT(sl_onLblLinkActivated(const QString&)));

    // General settings widgets
    connect(treeViewCombo, SIGNAL(currentIndexChanged(int)), SLOT(sl_onGeneralSettingsChanged()));
    connect(layoutCombo,   SIGNAL(currentIndexChanged(int)), SLOT(sl_onLayoutChanged(int)));
    
    connect(getTreeViewer(), SIGNAL(si_settingsChanged(TreeSettingsType)), SLOT(sl_onSettingsChanged(TreeSettingsType)));

    //Labels settings widgets
    connect(showNamesCheck,      SIGNAL(stateChanged(int)), SLOT(sl_onLabelsSettingsChanged()));
    connect(showDistancesCheck,  SIGNAL(stateChanged(int)), SLOT(sl_onLabelsSettingsChanged()));
    connect(alignLabelsCheck,    SIGNAL(stateChanged(int)), SLOT(sl_onLabelsSettingsChanged()));

    //Labels format widgets
    connect(labelsColorButton,   SIGNAL(clicked()),     SLOT(sl_labelsColorButton()));
    connect(boltAttrButton,      SIGNAL(clicked(bool)), SLOT(sl_textSettingsChanged()));
    connect(italicAttrButton,    SIGNAL(clicked(bool)), SLOT(sl_textSettingsChanged()));
    connect(underlineAttrButton, SIGNAL(clicked(bool)), SLOT(sl_textSettingsChanged()));
    connect(fontSizeSpinBox,     SIGNAL(valueChanged(int)), SLOT(sl_textSettingsChanged()));
    connect(fontComboBox,        SIGNAL(currentFontChanged(const QFont &)), SLOT(sl_textSettingsChanged()));

    //Branches settings widgets
    connect(widthSlider,   SIGNAL(valueChanged(int)), SLOT(sl_onGeneralSettingsChanged()));
    connect(heightSlider,  SIGNAL(valueChanged(int)), SLOT(sl_onGeneralSettingsChanged()));

    connect(branchesColorButton, SIGNAL(clicked()),         SLOT(sl_branchesColorButton()));
    connect(lineWeightSpinBox,   SIGNAL(valueChanged(int)), SLOT(sl_branchSettingsChanged()));
}

void TreeOptionsWidget::createGeneralSettingsWidgets(){
    QStringList items;
    items << tr("Rectangular") << tr("Circular") << tr("Unrooted");
    layoutCombo->addItems(items);

    treeViewCombo->addItem(TreeSettingsDialog::treeDefaultText());
    treeViewCombo->addItem(TreeSettingsDialog::treePhylogramText());
    treeViewCombo->addItem(TreeSettingsDialog::treeCladogramText());
    updateGeneralSettingsWidgets();
}

void TreeOptionsWidget::updateGeneralSettingsWidgets() {
    switch (treeSettings.type)
    {
    case TreeSettings::DEFAULT:
        treeViewCombo->setCurrentIndex(treeViewCombo->findText(TreeSettingsDialog::treeDefaultText()));
        break;
    case TreeSettings::PHYLOGRAM:
        treeViewCombo->setCurrentIndex(treeViewCombo->findText(TreeSettingsDialog::treePhylogramText()));
        break;
    case TreeSettings::CLADOGRAM:
        treeViewCombo->setCurrentIndex(treeViewCombo->findText(TreeSettingsDialog::treeCladogramText()));
        break;
    }

    TreeViewerUI::TreeLayout layout = getTreeViewer()->getTreeLayout();
    switch(layout) {
    case TreeViewerUI::TreeLayout_Rectangular:
        layoutCombo->setCurrentIndex(0);
        break;
    case TreeViewerUI::TreeLayout_Circular:
        layoutCombo->setCurrentIndex(1);
        break;
    case TreeViewerUI::TreeLayout_Unrooted:
        layoutCombo->setCurrentIndex(2);
        break;
    };
}

void TreeOptionsWidget::updateLabelsSettingsWidgets() {
    labelsSettings = getTreeViewer()->getLabelsSettings();

    showNamesCheck->setCheckState(labelsSettings.showNames ? Qt::Checked : Qt::Unchecked);
    showDistancesCheck->setCheckState(labelsSettings.showDistances ? Qt::Checked : Qt::Unchecked);
    alignLabelsCheck->setCheckState(labelsSettings.alignLabels ? Qt::Checked : Qt::Unchecked);

    textSettings = getTreeViewer()->getTextSettings();
}

void TreeOptionsWidget::updateFormatSettings() {
    //Update labels format settings widgets
    updateButtonColor(labelsColorButton, textSettings.textColor);

    fontSizeSpinBox->setValue(textSettings.textFont.pointSize());

    boltAttrButton->setCheckable(true);
    italicAttrButton->setCheckable(true);
    underlineAttrButton->setCheckable(true);

    boltAttrButton->setChecked(textSettings.textFont.bold());
    italicAttrButton->setChecked(textSettings.textFont.italic());
    underlineAttrButton->setChecked(textSettings.textFont.underline());

    fontComboBox->setCurrentFont(textSettings.textFont);
}

void TreeOptionsWidget::updateBranchSettings() {
    //Update branches size settings widgets
    widthSlider->setMinimum(10);
    widthSlider->setMaximum(50);

    heightSlider->setMinimum(1);
    heightSlider->setMaximum(20);

    heightSlider->setSliderPosition(treeSettings.height_coef);
    widthSlider->setSliderPosition(treeSettings.width_coef);

    heightSlider->setEnabled(getTreeViewer()->layoutIsRectangular());
    //Update branches pen settings
    lineWeightSpinBox->setMinimum(1);

    branchSettings = getTreeViewer()->getBranchSettings();
    lineWeightSpinBox->setValue(branchSettings.branchThickness);
    updateButtonColor(branchesColorButton, branchSettings.branchColor);
}

TreeViewerUI* TreeOptionsWidget::getTreeViewer() {
    SAFE_POINT(msa != NULL || treeViewer != NULL, QString("Invalid parameter in constructor TreeOptionsWidget"), NULL);
    return treeViewer != NULL ? treeViewer : msa->getUI()->getCurrentTree()->getTreeViewerUI();
}

void TreeOptionsWidget::sl_onGeneralSettingsChanged()
{
    treeSettings.height_coef = heightSlider->value();
    treeSettings.width_coef = widthSlider->value();
    
    if (treeViewCombo->currentText() == TreeSettingsDialog::treeDefaultText())
    {
        treeSettings.type = TreeSettings::DEFAULT;
    }
    else if (treeViewCombo->currentText() == TreeSettingsDialog::treePhylogramText())
    {
        treeSettings.type = TreeSettings::PHYLOGRAM;
    } else {
        SAFE_POINT(treeViewCombo->currentText() == TreeSettingsDialog::treeCladogramText() , QString("Unexpected tree type value in TreeOptionsWidget"),);
        treeSettings.type = TreeSettings::CLADOGRAM;
    }

    getTreeViewer()->updateSettings(treeSettings);
}

void TreeOptionsWidget::sl_textSettingsChanged() {
    QFont newFont = fontComboBox->currentFont();
    newFont.setPointSize(fontSizeSpinBox->value());

    newFont.setBold(boltAttrButton->isChecked());
    newFont.setItalic(italicAttrButton->isChecked());
    newFont.setUnderline(underlineAttrButton->isChecked());

    if(newFont != textSettings.textFont) {
        textSettings.textFont = newFont;
        getTreeViewer()->updateSettings(textSettings);
    }
}

void TreeOptionsWidget::sl_onLabelsSettingsChanged() {
    TreeLabelsSettings newSettings;
    newSettings.alignLabels = (Qt::Checked ==  alignLabelsCheck->checkState());
    newSettings.showNames = (Qt::Checked ==  showNamesCheck->checkState());
    newSettings.showDistances = (Qt::Checked ==  showDistancesCheck->checkState());
    if(labelsSettings != newSettings) {
        labelsSettings = newSettings;
        alignLabelsCheck->setEnabled(labelsSettings.showNames);

        getTreeViewer()->updateSettings(labelsSettings);
    }
}

void TreeOptionsWidget::sl_onLayoutChanged(int index) {
    heightSlider->setEnabled(false);
    lblHeightSlider->setEnabled(false);
    switch(index) {
        case 0:
            getTreeViewer()->setTreeLayout(TreeViewerUI::TreeLayout_Rectangular);
            heightSlider->setEnabled(true);
            lblHeightSlider->setEnabled(true);
            break;
        case 1:
            getTreeViewer()->setTreeLayout(TreeViewerUI::TreeLayout_Circular);
            break;
        case 2:
            getTreeViewer()->setTreeLayout(TreeViewerUI::TreeLayout_Unrooted);
            break;
    }
}

void TreeOptionsWidget::sl_labelsColorButton() {
    QColor newColor = QColorDialog::getColor(textSettings.textColor, this);
    if (newColor.isValid()) {
       textSettings.textColor = newColor;
        labelsColorButton->setStyleSheet(COLOR_BOX_STYLE.arg(textSettings.textColor.name()));
        sl_textSettingsChanged();
    }
}

void TreeOptionsWidget::sl_branchesColorButton() {
    QColor newColor = QColorDialog::getColor(branchSettings.branchColor, this);
    if (newColor.isValid()) {
        branchSettings.branchColor = newColor;
        branchesColorButton->setStyleSheet(COLOR_BOX_STYLE.arg(branchSettings.branchColor.name()));
        sl_branchSettingsChanged();
    }
}

void TreeOptionsWidget::sl_branchSettingsChanged() {
    branchSettings.branchThickness = lineWeightSpinBox->value();
    getTreeViewer()->updateSettings(branchSettings);
}

void TreeOptionsWidget::sl_onLblLinkActivated(const QString& link) {
    if(SHOW_FONT_OPTIONS_LINK == link) {
        showFontSettings = !showFontSettings;
        QString labelText = showFontSettings ? "Hide font settings" : "Show font settings";
        updateShowFontOpLabel(labelText);
        fontSettingsWidget->setVisible(showFontSettings);
        return;
    }
    if(SHOW_PEN_OPTIONS_LINK == link) {
        showPenSettings = !showPenSettings;
        QString labelText = showPenSettings ? "Hide pen settings" : "Show pen settings";
        updateShowPenOpLabel(labelText);
        penGroup->setVisible(showPenSettings);
    }
}

void TreeOptionsWidget::sl_onSettingsChanged(TreeSettingsType settingsType) {
    TreeViewerUI* treeViewer = getTreeViewer();
    CHECK(NULL != treeViewer,);
    switch(settingsType) {
        case GENERAL_SETTINGS:
            treeSettings = treeViewer->getTreeSettings();
            updateGeneralSettingsWidgets();
            updateBranchSettings();
            break;
        case LABELS_SETTINGS:
            labelsSettings = treeViewer->getLabelsSettings();
            updateLabelsSettingsWidgets();
            break;
        case TEXT_FORMAT:
            textSettings = treeViewer->getTextSettings();
            updateFormatSettings();
            break;
        case BRANCHES_SETTINGS:
            branchSettings = treeViewer->getBranchSettings();
            updateBranchSettings();
            updateButtonColor(branchesColorButton, branchSettings.branchColor);
            break;
        default:
            //Can not change this type of settings
            break;
    }
}

void TreeOptionsWidget::updateButtonColor(QPushButton* button, const QColor& newColor ) {
    button->setStyleSheet(COLOR_BOX_STYLE.arg(newColor.name()));
}

void TreeOptionsWidget::updateShowFontOpLabel(QString newText) {
    newText = QString("<a href=\"%1\" style=\"color: palette(shadow)\">").arg(SHOW_FONT_OPTIONS_LINK)/*.arg(L10N::showMoreColorLabelStr())*/
        + newText
        + QString("</a>");

    lblFontSettings->setText(newText);
    lblFontSettings->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);
}

void TreeOptionsWidget::updateShowPenOpLabel(QString newText) {
    newText = QString("<a href=\"%1\" style=\"color: %2\">").arg(SHOW_PEN_OPTIONS_LINK).arg(L10N::showMoreColorLabelStr())
        + newText
        + QString("</a>");

    lblPenSettings->setText(newText);
    lblPenSettings->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);
}



AddTreeWidget::AddTreeWidget(MSAEditor* msa)
    : editor(msa), openTreeButton(NULL), buildTreeButton(NULL), addTreeHint(NULL) 
{
    setObjectName( "AddTreeWidget" );
    QVBoxLayout* mainLayout = initLayout(this);
    mainLayout->setSpacing(0);

    addTreeHint = new QLabel(tr("There are no displayed trees so settings are hidden."), this);
    addTreeHint->setWordWrap(true);

    mainLayout->addWidget(addTreeHint);

    QHBoxLayout* buttonLayout = new QHBoxLayout(this);
    buttonLayout->setContentsMargins(0, 5, 0, 0);

    openTreeButton = new QPushButton(QIcon(":ugene/images/advanced_open.png"), tr("Open tree"), this);
    openTreeButton->setMinimumWidth(20);
    buttonLayout->addWidget(openTreeButton);
    openTreeButton->setObjectName( "OpenTreeButton" );

    QSpacerItem* horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    buttonLayout->addSpacerItem(horizontalSpacer);

    buildTreeButton = new QPushButton(QIcon(":core/images/phylip.png"), tr("Build tree"), this);
    buildTreeButton->setMinimumWidth(20);
    buttonLayout->addWidget(buildTreeButton);
    openTreeButton->setObjectName( "BuildTreeButton" );

    mainLayout->addLayout(buttonLayout);

    connect(openTreeButton, SIGNAL(clicked()), SLOT(sl_onOpenTreeTriggered()));
    connect(buildTreeButton, SIGNAL(clicked()), SLOT(sl_onBuildTreeTriggered()));
}
void AddTreeWidget::sl_onOpenTreeTriggered() {
    editor->getTreeManager()->openTreeFromFile();
}

void AddTreeWidget::sl_onBuildTreeTriggered() {
    editor->getTreeManager()->buildTreeWithDialog();
}

}
