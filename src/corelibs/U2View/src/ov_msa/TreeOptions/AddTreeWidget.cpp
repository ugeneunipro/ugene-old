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

#include "AddTreeWidget.h"

#include <U2Gui/ShowHideSubgroupWidget.h>

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Settings.h>
#include <U2Core/AppResources.h>
#include <U2Core/PluginModel.h>

#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

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

AddTreeWidget::AddTreeWidget(const MAlignmentObject* mobj):msa(_mobj->getMAlignment()), mobj(_mobj){
    createGroups();
}
void AddTreeWidget::createGroups() {
    QVBoxLayout* mainLayout = initLayout(this);
    mainLayout->setSpacing(0);

    QWidget * similarityGroup = new ShowHideSubgroupWidget("TREE", tr("Trees settings"), createTreesSettings(), true);
    mainLayout->addWidget(similarityGroup);
    QWidget * visualGroup = new ShowHideSubgroupWidget("TREE_VISUAL", tr("Labels formating"), createLabelsFormatSettings(), true);
    mainLayout->addWidget(visualGroup);
    QWidget * branchGroup = new ShowHideSubgroupWidget("BRANCH_SETTINGS", tr("Branches settings"), createBranchSettings(), true);
    mainLayout->addWidget(branchGroup);
}

QWidget* AddTreeWidget::createSettingsWidget() {
    QWidget* group = new QWidget();
    QVBoxLayout* groupLayout = initLayout(group);
    algorithmBox = new QComboBox(group);
    groupLayout->addItem(algorithmBox);
    
    const GUrl& msaURL = mobj->getDocument()->getURL();
    GUrl url = GUrlUtils::rollFileName(msaURL.dirPath() + "/" + msaURL.baseFileName() + ".nwk", DocumentUtils::getNewDocFileNameExcludesHint());
    
    fileNameEdit = new QLineEdit(group);
    groupLayout->addItem(fileNameEdit);

    fileNameEdit->setText(url.getURLString());
    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    algorithmBox->addItems(registry->getNameList());

    connect(browseButton, SIGNAL(clicked()), SLOT(sl_browseClicked()));
    connect(algorithmBox, SIGNAL(currentIndexChanged ( int )), SLOT(sl_comboIndexChaged(int)));
    
    int itemIndex = algorithmBox->count()-1;
    assert(itemIndex >= 0);

    int curIndex = algorithmBox->currentIndex();
    if(itemIndex == curIndex){ //cause the signal currentIndexChanged isn't sent in this case
        PhyTreeGenerator* generator = registry->getGenerator(algorithmBox->itemText(curIndex));
        generator->setupCreatePhyTreeUI(this, msa);
        verticalLayout->activate();
    }
    else {
        algorithmBox->setCurrentIndex(i);
    }
}

void CreatePhyTreeDialogController::insertContrWidget( int pos, CreatePhyTreeWidget* widget )
{
    QWidget* contrWidget = new QWidget(this);
    QVBoxLayout* contrLayout = new QVBoxLayout(contrWidget);

    QLayout* mainLayout = widget->layout();
    QStack<QLayout*> layoutStack;
    layoutStack.push(mainLayout);
    do{
        QLayout* curLayout = layoutStack.pop();
        int count = curLayout->count();
        for(int i = 0; i < count; i++) {
            QLayoutItem* curItem = curLayout->itemAt(i);
            QLayout* layout = qobject_cast<QLayout*>(curItem);
            if(NULL != layout) {
                layoutStack.push(layout);
                continue;
            }
            QGroupBox* group = qobject_cast<QGroupBox*>(curItem);
            if(NULL != group) {
                layoutStack.push();
                continue;
            }
            contrLayout->addWidget(curItem->widget());
        }
    }while(!layoutStack.isEmpty());

    contrWidget->setLayout(contrLayout);
    childWidgets.append(contrWidget);

}

void CreatePhyTreeDialogController::clearContrWidgets(){
    foreach(QWidget* w, childWidgets){
        // adjust sizes
        setMinimumHeight(minimumHeight() - w->minimumHeight());
        w->hide();
        delete w;
    }    
    childWidgets.clear();
    adjustSize();
}

void CreatePhyTreeDialogController::sl_browseClicked()
{
    GUrl oldUrl = ui->fileNameEdit->text(); 
    QString path;
    LastUsedDirHelper lod;
    if (oldUrl.isEmpty()) {
        path = lod.dir;
    } else {
        path = oldUrl.getURLString();
    }
    GUrl newUrl = QFileDialog::getSaveFileName(this, "Choose file name", path,"Newick format (*.nwk)");

    if (newUrl.isEmpty()) {
        return;
    }
    ui->fileNameEdit->setText(newUrl.getURLString());
    lod.url = newUrl.getURLString();
}

void CreatePhyTreeDialogController::sl_comboIndexChaged(int ){
    clearContrWidgets();
    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    PhyTreeGenerator* generator = registry->getGenerator(ui->algorithmBox->currentText());
    generator->setupCreatePhyTreeUI(this, msa);
    ui->verticalLayout->activate(); 
}
}
