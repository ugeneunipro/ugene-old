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

#include "TreeOptionsWidget.h"

#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Core/AppContext.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include "U2View/MSAEditorDataList.h"
#include <U2View/TreeViewer.h>
#include <U2View/MSAEditorMultiTreeViewer.h>

namespace U2 {

static const int ITEMS_SPACING = 10;
static const int TITLE_SPACING = 5;

static inline QVBoxLayout * initLayout(QWidget * w) {
    QVBoxLayout * layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignTop);

    w->setLayout(layout);
    return layout;
}

TreeOptionsWidget::TreeOptionsWidget(MSAEditor* m):msa(m), settings(NULL), treeViewer(NULL){
    SAFE_POINT(NULL != msa, QString("Invalid parameter were passed into constructor TreeOptionsWidget"), );

    settings = &(getTreeViewer()->getTreeSettings());
    createGroups();
}
TreeOptionsWidget::TreeOptionsWidget(TreeViewer* tree): msa(NULL), settings(NULL), treeViewer(tree->ui){
    SAFE_POINT(NULL != treeViewer, QString("Invalid parameter were passed into constructor TreeOptionsWidget"), );

    settings = &(treeViewer->getTreeSettings());
    createGroups();
}
void TreeOptionsWidget::createGroups() {
    QVBoxLayout* mainLayout = initLayout(this);
    mainLayout->setSpacing(0);

    if(NULL != msa) {
        QWidget * syncGroup = new ShowHideSubgroupWidget("TREE_SYNC", tr("Sync settings"), createTreeSyncSettings(), true);
        mainLayout->addWidget(syncGroup);
    }
    QWidget * similarityGroup = new ShowHideSubgroupWidget("TREE", tr("Trees settings"), createTreesSettings(), true);
    mainLayout->addWidget(similarityGroup);
    QWidget * visualGroup = new ShowHideSubgroupWidget("TREE_VISUAL", tr("Labels formating"), createLabelsFormatSettings(), true);
    mainLayout->addWidget(visualGroup);
    QWidget * branchGroup = new ShowHideSubgroupWidget("BRANCH_SETTINGS", tr("Branches settings"), createBranchSettings(), true);
    mainLayout->addWidget(branchGroup);
}
TreeViewerUI* TreeOptionsWidget::getTreeViewer() {
    SAFE_POINT(msa != NULL || treeViewer != NULL, QString("Invalid parameter in constructor TreeOptionsWidget"), NULL);
    return treeViewer != NULL ? treeViewer : msa->getUI()->getCurrentTree()->ui;
}

void TreeOptionsWidget::sl_onSettingsChanged()
{
    newSettings.height_coef = heightSlider->value();
    newSettings.width_coef = widthSlider->value();

    if (treeViewCombo->currentText() == TreeSettingsDialog::treePhylogramText())
    {
        newSettings.type = TreeSettings::PHYLOGRAM;
    } else if (treeViewCombo->currentText() == TreeSettingsDialog::treeCladogramText()) {
        newSettings.type = TreeSettings::CLADOGRAM;
    } else {
        SAFE_POINT(false , QString("Unexpected tree type value in TreeOptionsWidget"),);
    }

    getTreeViewer()->updateSettings(newSettings);
}
QWidget* TreeOptionsWidget::createTreesSettings(){
    QWidget * group = new QWidget(this);

    QVBoxLayout* groupLayout = initLayout(this);
    
    QLabel* layoutLabel = new QLabel(tr("Tree layout"), group);
    groupLayout->addWidget(layoutLabel);
    layoutCombo = new QComboBox(group);
    QStringList items;
    items << tr("Rectangular") << tr("Circular") << tr("Unrooted");
    layoutCombo->addItems(items);
    groupLayout->addWidget(layoutCombo);

    QLabel* widthLabel = new QLabel(tr("Width"), group);
    groupLayout->addWidget(widthLabel);
    widthSlider = new QSlider(Qt::Horizontal, group);
    widthSlider->setMinimum(10);
    widthSlider->setMaximum(50);
    groupLayout->addWidget(widthSlider);

    QLabel* heightLabel = new QLabel(tr("Height"), group);
    groupLayout->addWidget(heightLabel);
    heightSlider = new QSlider(Qt::Horizontal, group);
    heightSlider->setMinimum(1);
    heightSlider->setMaximum(20);
    groupLayout->addWidget(heightSlider);

    QLabel* viewLabel = new QLabel(tr("Tree view"), group);
    groupLayout->addWidget(viewLabel);
    treeViewCombo = new QComboBox(group);
    groupLayout->addWidget(treeViewCombo);

    groupLayout->addWidget(createLabelsSettings());

    group->setLayout(groupLayout);

    if(NULL != settings) {
        heightSlider->setSliderPosition(settings->height_coef);
        widthSlider->setSliderPosition(settings->width_coef);

        heightSlider->setEnabled(getTreeViewer()->layoutIsRectangular());

        treeViewCombo->addItem(TreeSettingsDialog::treePhylogramText());
        treeViewCombo->addItem(TreeSettingsDialog::treeCladogramText());

        switch ( settings->type )
        {
        case TreeSettings::PHYLOGRAM:
            treeViewCombo->setCurrentIndex(treeViewCombo->findText(TreeSettingsDialog::treePhylogramText()));
            break;
        case TreeSettings::CLADOGRAM:
            treeViewCombo->setCurrentIndex(treeViewCombo->findText(TreeSettingsDialog::treeCladogramText()));
            break;
        default:
            SAFE_POINT(false , QString("Unexpected tree type value in TreeOptionsWidget"), NULL);
            break;
        }
    }
    else {
        group->setEnabled(false);
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

    connect(widthSlider,   SIGNAL(valueChanged(int)),         this, SLOT(sl_onSettingsChanged()));
    connect(heightSlider,  SIGNAL(valueChanged(int)),         this, SLOT(sl_onSettingsChanged()));
    connect(treeViewCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_onSettingsChanged()));
    connect(layoutCombo,   SIGNAL(currentIndexChanged(int)), this, SLOT(sl_onLayoutChanged(int)));

    return group;
}
QWidget* TreeOptionsWidget::createLabelsSettings() {
    QWidget * group = new QWidget(this);

    QVBoxLayout* groupLayout = initLayout(this);

    showNamesCheck = new QCheckBox(tr("Show names"), group);
    groupLayout->addWidget(showNamesCheck);
    showDistancesCheck = new QCheckBox(tr("Show distances"), group);
    groupLayout->addWidget(showDistancesCheck);
    alignLabelsCheck = new QCheckBox(tr("Align labels"), group);
    groupLayout->addWidget(alignLabelsCheck);

    group->setLayout(groupLayout);
    
    labelsSettings = getTreeViewer()->getLabelsSettings();

    showNamesCheck->setCheckState(labelsSettings.showNames ? Qt::Checked : Qt::Unchecked);
    showDistancesCheck->setCheckState(labelsSettings.showDistances ? Qt::Checked : Qt::Unchecked);
    alignLabelsCheck->setCheckState(labelsSettings.alignLabels ? Qt::Checked : Qt::Unchecked);

    connect(showNamesCheck,      SIGNAL(stateChanged(int)), this, SLOT(sl_onLabelsSettingsChanged()));
    connect(showDistancesCheck,  SIGNAL(stateChanged(int)), this, SLOT(sl_onLabelsSettingsChanged()));
    connect(alignLabelsCheck,    SIGNAL(stateChanged(int)), this, SLOT(sl_onLabelsSettingsChanged()));

    return group;
}

void TreeOptionsWidget::sl_textSettingsChanged()
{
    textSettings.textFont = fontComboBox->currentFont();
    textSettings.textFont.setPointSize(sizeSpinBox->value());

    textSettings.textFont.setBold(boldToolButton->isChecked());
    textSettings.textFont.setItalic(italicToolButton->isChecked());
    textSettings.textFont.setUnderline(underlineToolButton->isChecked());
    textSettings.textFont.setOverline(overlineToolButton->isChecked());

    getTreeViewer()->updateSettings(textSettings);
}

QWidget* TreeOptionsWidget::createBranchSettings() {
    QWidget *group = new QWidget(this);

    QVBoxLayout* groupLayout = initLayout(this);
 
    QLabel* colorLabel = new QLabel("Branch color", group);
    groupLayout->addWidget(colorLabel);
    branchColorButton = new QPushButton(group);
    groupLayout->addWidget(branchColorButton);

    QLabel* lineWeight = new QLabel("Line weight", group);
    groupLayout->addWidget(lineWeight);
    thicknessSpinBox = new QSpinBox();
    thicknessSpinBox->setMinimum(1);
    groupLayout->addWidget(thicknessSpinBox);

    group->setLayout(groupLayout);

    branchSettings = getTreeViewer()->getBranchSettings();
    thicknessSpinBox->setValue(branchSettings.branchThickness);
    updateButtonColor(branchColorButton, branchSettings.branchColor);

    connect(branchColorButton, SIGNAL(clicked()),         SLOT(sl_branchColorButton()));
    connect(thicknessSpinBox,  SIGNAL(valueChanged(int)), SLOT(sl_branchSettingsChanged()));

    return group;
}
void TreeOptionsWidget::sl_onLabelsSettingsChanged() {
    labelsSettings.alignLabels = (Qt::Checked ==  alignLabelsCheck->checkState());
    labelsSettings.showNames = (Qt::Checked ==  showNamesCheck->checkState());
    labelsSettings.showDistances = (Qt::Checked ==  showDistancesCheck->checkState());

    getTreeViewer()->updateSettings(labelsSettings);
}
void TreeOptionsWidget::sl_onLayoutChanged(int index) {
    switch(index) {
        case 0:
            getTreeViewer()->setTreeLayout(TreeViewerUI::TreeLayout_Rectangular);
            break;
        case 1:
            getTreeViewer()->setTreeLayout(TreeViewerUI::TreeLayout_Circular);
            break;
        case 2:
            getTreeViewer()->setTreeLayout(TreeViewerUI::TreeLayout_Unrooted);
            break;
    }
}

void TreeOptionsWidget::updateVisualSettings()
{
    textSettings = getTreeViewer()->getTextSettings();
    
    updateButtonColor(colorButton, textSettings.textColor);


    fontComboBox->setCurrentFont(textSettings.textFont);
    sizeSpinBox->setValue(textSettings.textFont.pointSize());

    boldToolButton->setChecked(textSettings.textFont.bold());
    italicToolButton->setChecked(textSettings.textFont.italic());
    underlineToolButton->setChecked(textSettings.textFont.underline());
    overlineToolButton->setChecked(textSettings.textFont.overline());

    overlineToolButton->setVisible(false);
}
void TreeOptionsWidget::sl_colorButton() {
    QColor newColor = QColorDialog::getColor(textSettings.textColor, this);
    if (newColor.isValid()) {
       textSettings.textColor = newColor;
        static const QString COLOR_STYLE("QPushButton { background-color : %1;}");
        colorButton->setStyleSheet(COLOR_STYLE.arg(textSettings.textColor.name()));
        sl_textSettingsChanged();
    }
}
void TreeOptionsWidget::sl_branchColorButton() {
    QColor newColor = QColorDialog::getColor(branchSettings.branchColor, this);
    if (newColor.isValid()) {
        branchSettings.branchColor = newColor;
        static const QString COLOR_STYLE("QPushButton { background-color : %1;}");
        branchColorButton->setStyleSheet(COLOR_STYLE.arg(branchSettings.branchColor.name()));
        sl_branchSettingsChanged();
    }
}

QWidget* TreeOptionsWidget::createLabelsFormatSettings()
{
    QWidget* formatGroup  = new QWidget(this);
    QVBoxLayout* formatLayout = initLayout(formatGroup);

    QLabel* colorLabel = new QLabel(tr("Color"), formatGroup);
    formatLayout->addWidget(colorLabel);
    colorButton = new QPushButton(formatGroup);
    formatLayout->addWidget(colorButton);

    QLabel* fontLabel = new QLabel(tr("Font"), formatGroup);
    formatLayout->addWidget(fontLabel);
    fontComboBox = new QFontComboBox(formatGroup);
    formatLayout->addWidget(fontComboBox);

    QLabel* sizeLabel = new QLabel(tr("Size"), formatGroup);
    formatLayout->addWidget(sizeLabel);
    sizeSpinBox = new QSpinBox(formatGroup);
    sizeSpinBox->setMinimum(1);
    formatLayout->addWidget(sizeSpinBox);

    QWidget* attributesWidget = new QWidget(formatGroup);
    QHBoxLayout* attributesLayout = new QHBoxLayout(formatGroup);
    QLabel* attributesLabel = new QLabel(tr("Attributes"), formatGroup);
    attributesLayout->addWidget(attributesLabel);

    boldToolButton = new QToolButton(formatGroup);
    QFont boldFont;
    boldFont.setBold(true);
    boldFont.setWeight(75);
    boldToolButton->setFont(boldFont);
    boldToolButton->setCheckable(true);

    attributesLayout->addWidget(boldToolButton);

    italicToolButton = new QToolButton(formatGroup);
    QFont italicFont;
    italicFont.setItalic(true);
    italicToolButton->setFont(italicFont);
    italicToolButton->setCheckable(true);

    attributesLayout->addWidget(italicToolButton);

    underlineToolButton = new QToolButton(formatGroup);
    QFont underlineFont;
    underlineFont.setUnderline(true);
    underlineToolButton->setFont(underlineFont);
    underlineToolButton->setCheckable(true);

    attributesLayout->addWidget(underlineToolButton);

    overlineToolButton = new QToolButton(formatGroup);
    overlineToolButton->setObjectName(QString::fromUtf8("overlineToolButton"));
    QFont strikeOutFont;
    strikeOutFont.setStrikeOut(true);
    overlineToolButton->setFont(strikeOutFont);
    overlineToolButton->setCheckable(true);

    attributesLayout->addWidget(overlineToolButton);

    boldToolButton->setText(tr("B"));
    italicToolButton->setText(tr("I"));
    underlineToolButton->setText(tr("U"));
    overlineToolButton->setText(tr("S"));

    attributesWidget->setLayout(attributesLayout);
    formatLayout->addWidget(attributesWidget);
    formatGroup->setLayout(formatLayout);

    updateVisualSettings();

    connect(colorButton,         SIGNAL(clicked()),     SLOT(sl_colorButton()));
    connect(boldToolButton,      SIGNAL(clicked(bool)), SLOT(sl_textSettingsChanged()));
    connect(italicToolButton,    SIGNAL(clicked(bool)), SLOT(sl_textSettingsChanged()));
    connect(underlineToolButton, SIGNAL(clicked(bool)), SLOT(sl_textSettingsChanged()));
    connect(overlineToolButton,  SIGNAL(clicked(bool)), SLOT(sl_textSettingsChanged()));
    connect(sizeSpinBox,         SIGNAL(valueChanged(int)), SLOT(sl_textSettingsChanged()));
    connect(fontComboBox,        SIGNAL(currentIndexChanged(int)), SLOT(sl_textSettingsChanged()));

    return formatGroup;
}

QWidget* TreeOptionsWidget::createTreeSyncSettings() {
    QWidget* syncGroup = new QWidget(this);
    QVBoxLayout* syncLayout = initLayout(syncGroup);

    currentTree = new QComboBox(syncGroup);
    syncLayout->addWidget(currentTree);

    syncSizesCheck = new QCheckBox(tr("Synchronize sizes"), syncGroup);
    syncLayout->addWidget(syncSizesCheck);
    
    sortSeqByTreeCheck = new QCheckBox(tr("Sort sequences by tree"), syncGroup);
    syncLayout->addWidget(sortSeqByTreeCheck);

    syncCollapsingCheck = new QCheckBox(tr("Synchronize collapsing"), syncGroup);
    syncLayout->addWidget(syncCollapsingCheck);

    syncGroup->setLayout(syncLayout);

    updateSyncSettings();

    connect(syncSizesCheck,      SIGNAL(stateChanged(int)), SLOT(sl_syncSettingsChanged()));
    connect(sortSeqByTreeCheck,  SIGNAL(stateChanged(int)), SLOT(sl_syncSettingsChanged()));
    connect(syncCollapsingCheck, SIGNAL(stateChanged(int)), SLOT(sl_syncSettingsChanged()));

    return syncGroup ;
}
void TreeOptionsWidget::sl_branchSettingsChanged()
{
    branchSettings.branchThickness = thicknessSpinBox->value();
    getTreeViewer()->updateSettings(branchSettings);
}

void TreeOptionsWidget::updateButtonColor(QPushButton* button, const QColor& newColor ) {
    static const QString COLOR_STYLE("QPushButton { background-color : %1;}");
    button->setStyleSheet(COLOR_STYLE.arg(newColor.name()));
}

void TreeOptionsWidget::sl_syncSettingsChanged() {

}

void TreeOptionsWidget::updateSyncSettings() {
    syncSettings= msa->getUI()->getCurrentTree()->getSyncSettings();

    syncSizesCheck->setCheckState(syncSettings.syncScale ? Qt::Checked : Qt::Unchecked);
    sortSeqByTreeCheck->setCheckState(syncSettings.syncSequencesOrder ? Qt::Checked : Qt::Unchecked);
    syncCollapsingCheck->setCheckState(syncSettings.syncCollapsing ? Qt::Checked : Qt::Unchecked);

    currentTree->addItems(msa->getUI()->getMultiTreeViewer()->getTreeNames());
}

}