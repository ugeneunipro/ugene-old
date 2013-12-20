#include "CreatePhyTreeDialogController.h"
#include "ui/ui_CreatePhyTreeDialog.h"

#include "CreatePhyTreeWidget.h"

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
#include <U2Core/TmpDirChecker.h>

#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

#include <U2Gui/LastUsedDirHelper.h>

#include <U2View/LicenseDialog.h>

#include <QtGui/qfiledialog.h>
#include <QtGui/qmessagebox.h>

namespace U2{

CreatePhyTreeDialogController::CreatePhyTreeDialogController(QWidget* parent, const MAlignmentObject* mobj, CreatePhyTreeSettings& _settings)
: QDialog(parent), msa(mobj->getMAlignment()), settings(_settings){
    ui = new Ui_CreatePhyTree;
    ui->setupUi(this);
    ui->verticalLayout->getContentsMargins ( NULL, NULL, &rightMargin, NULL );
    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    QStringList nameList = registry->getNameList();
    for(int i=0; i<nameList.length(); i++){
        QString item = nameList[i];
        ui->algorithmBox->addItem(item);
    }
    const GUrl& msaURL = mobj->getDocument()->getURL();
    GUrl url = GUrlUtils::rollFileName(msaURL.dirPath() + "/" + msaURL.baseFileName() + ".nwk", DocumentUtils::getNewDocFileNameExcludesHint());
    connect(ui->okButton, SIGNAL(clicked()), SLOT(sl_okClicked()));
    connect(ui->browseButton, SIGNAL(clicked()), SLOT(sl_browseClicked()));
    connect(ui->algorithmBox, SIGNAL(currentIndexChanged ( int )), SLOT(sl_comboIndexChaged(int)));
    connect(ui->storeSettings, SIGNAL(clicked()), SLOT(sl_onStoreSettings()));
    connect(ui->restoreSettings, SIGNAL(clicked()), SLOT(sl_onRestoreDefault()));
    connect(ui->displayWithAlignmentEditor, SIGNAL(toggled(bool)), SLOT(sl_onDispayWithMSAClicked(bool)));
    
    ui->fileNameEdit->setText(url.getURLString());
    
    int itemIndex = ui->algorithmBox->count()-1;
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
    QFileInfo fileInfo(fileName);
    if(!TmpDirChecker::checkWritePermissions(fileInfo.path())) {
        QMessageBox::warning(this, tr("Warning"), tr("You don't have permission to write to the folder"));
        ui->fileNameEdit->setFocus();
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
