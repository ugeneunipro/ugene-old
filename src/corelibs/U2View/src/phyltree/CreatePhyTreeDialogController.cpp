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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#endif

#include <U2Algorithm/PhyTreeGeneratorRegistry.h>
#include <U2Algorithm/SubstMatrixRegistry.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/PluginModel.h>
#include <U2Core/Settings.h>
#include <U2Core/TmpDirChecker.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/U2FileDialog.h>

#include <U2View/LicenseDialog.h>

#include "CreatePhyTreeWidget.h"
#include "CreatePhyTreeDialogController.h"
#include "ui/ui_CreatePhyTreeDialog.h"

namespace U2{

CreatePhyTreeDialogController::CreatePhyTreeDialogController(QWidget* parent, const MAlignmentObject* mobj, CreatePhyTreeSettings& _settings)
: QDialog(parent), msa(mobj->getMAlignment()), settings(_settings){
    ui = new Ui_CreatePhyTree;
    ui->setupUi(this);
    QMap<QString,QString> helpPagesMap;
    helpPagesMap.insert("PHYLIP Neighbor Joining","4227494");
    helpPagesMap.insert("MrBayes","4227496");
    helpPagesMap.insert("PhyML Maximum Likelihood","4227492");
    new ComboboxDependentHelpButton(this, ui->buttonBox, ui->algorithmBox, helpPagesMap);
    //new HelpButton(this, ui->buttonBox, "4227492");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Build"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    ui->verticalLayout->getContentsMargins ( NULL, NULL, &rightMargin, NULL );
    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    QStringList nameList = registry->getNameList();
    for(int i=0; i<nameList.length(); i++){
        QString item = nameList[i];
        ui->algorithmBox->addItem(item);
    }

    QString url = GUrlUtils::getNewLocalUrlByExtention(mobj->getDocument()->getURLString(), mobj->getGObjectName(), ".nwk", "");

    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);

    connect(okButton, SIGNAL(clicked()), SLOT(sl_okClicked()));
    connect(ui->browseButton, SIGNAL(clicked()), SLOT(sl_browseClicked()));
    connect(ui->algorithmBox, SIGNAL(currentIndexChanged ( int )), SLOT(sl_comboIndexChaged(int)));
    connect(ui->storeSettings, SIGNAL(clicked()), SLOT(sl_onStoreSettings()));
    connect(ui->restoreSettings, SIGNAL(clicked()), SLOT(sl_onRestoreDefault()));
    connect(ui->displayWithAlignmentEditor, SIGNAL(toggled(bool)), SLOT(sl_onDispayWithMSAClicked(bool)));

    ui->fileNameEdit->setText(url);

    int itemIndex = nameList.indexOf("PHYLIP Neighbor Joining");
    if(itemIndex < 0) {
        itemIndex = ui->algorithmBox->count()-1;
    }
    assert(itemIndex >= 0);
    //QString algName = AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + "/algorithm", ui->algorithmBox->itemText(itemIndex)).toString();
    QString algName = ui->algorithmBox->itemText(itemIndex);

    for(int i = 0; i<ui->algorithmBox->count(); i++){
        if(ui->algorithmBox->itemText(i) == algName){
            if(i==0){ //cause the signal currentIndexChanged isn't sent in this case
                PhyTreeGenerator* generator = registry->getGenerator(ui->algorithmBox->itemText(0));
                generator->setupCreatePhyTreeUI(this, msa);
                ui->verticalLayout->activate();
            }
            ui->algorithmBox->setCurrentIndex(i);
            break;
        }
    }
    ui->displayWithAlignmentEditor->setChecked(true);
    ui->syncCheckBox->setCheckState(Qt::Checked);

}

void CreatePhyTreeDialogController::sl_okClicked(){

    settings.algorithmId = ui->algorithmBox->currentText();

    //Check license
    if (settings.algorithmId == "PHYLIP Neighbor Joining"){//This bad hack :(
        QList<Plugin*> plugins=AppContext::getPluginSupport()->getPlugins();
        foreach (Plugin* plugin, plugins){
            if(plugin->getName() == "PHYLIP"){
                if(!plugin->isLicenseAccepted()){
                    LicenseDialog licenseDialog(plugin);
                    int ret = licenseDialog.exec();
                    if(ret != QDialog::Accepted){
                        return;
                    }
                }
                break;
            }
        }
    }

    QString fileName = ui->fileNameEdit->text();
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please, input the file name."));
        ui->fileNameEdit->setFocus();
        return;
    }
    settings.fileUrl = fileName;

    U2OpStatus2Log os;
    GUrlUtils::validateLocalFileUrl(GUrl(fileName), os);
    if (os.hasError()) {
        QMessageBox::warning(this, tr("Error"), tr("Please, change the output file.") + "\n" + os.getError());
        ui->fileNameEdit->setFocus(Qt::MouseFocusReason);
        return;
    }

    settings.displayWithAlignmentEditor = ui->displayWithAlignmentEditor->isChecked();

    foreach (CreatePhyTreeWidget* widget, childWidgets) {
        widget->fillSettings(settings);
    }

    //checking
    QString msg;
    foreach (CreatePhyTreeWidget* widget, childWidgets) {
        if(!widget->checkSettings(msg, settings)){
            if(!msg.isEmpty()){
                QMessageBox::warning(this, tr("Warning"), msg);
            }
            return;
        }
    }
    bool memCheckOk = true;
    msg.clear();

    foreach (CreatePhyTreeWidget* widget, childWidgets) {
        if(!widget->checkMemoryEstimation(msg, msa, settings)){
            memCheckOk = false;
            break;
        }
    }

    if(!memCheckOk){
        QMessageBox mb(QMessageBox::Warning, tr("Warning"), msg, QMessageBox::Ok|QMessageBox::Cancel);
        if(mb.exec() == QMessageBox::Ok){
            QDialog::accept();
        }
    }else{
        QDialog::accept();
    }
}

void CreatePhyTreeDialogController::insertContrWidget( int pos, CreatePhyTreeWidget* widget )
{
    ui->verticalLayout->insertWidget(pos, widget);
    childWidgets.append(widget);

    // adjust sizes
    setMinimumHeight(widget->minimumHeight() + minimumHeight());
    if (minimumWidth() < widget->minimumWidth()) {
        setMinimumWidth(widget->minimumWidth() + rightMargin*2);
    };
    widget->show();
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
    GUrl newUrl = U2FileDialog::getSaveFileName(this, "Choose file name", path,"Newick format (*.nwk)");

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

void CreatePhyTreeDialogController::sl_onDispayWithMSAClicked(bool checked) {
     ui->syncCheckBox->setEnabled(checked);
}

CreatePhyTreeDialogController::~CreatePhyTreeDialogController()
{
    delete ui;
}

void CreatePhyTreeDialogController::sl_onStoreSettings(){
    //AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + "/algorithm", ui->algorithmBox->currentText());

    foreach (CreatePhyTreeWidget* widget, childWidgets) {
        widget->storeSettings();
    }
}
void CreatePhyTreeDialogController::sl_onRestoreDefault(){
    //AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + "/algorithm");
//     int itemIndex = ui->algorithmBox->count()-1;
//     assert(itemIndex >= 0);
//     ui->algorithmBox->setCurrentIndex(itemIndex);

    foreach (CreatePhyTreeWidget* widget, childWidgets) {
        widget->restoreDefault();
    }
}

}
