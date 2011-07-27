#include "CreatePhyTreeDialogController.h"
#include "ui/ui_CreatePhyTreeDialog.h"

#include "CreatePhyTreeWidget.h"
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Gui/DialogUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Settings.h>
#include <U2Core/AppResources.h>

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
        ui->algorithmBox->addItem(nameList[i]);
    }
    const GUrl& msaURL = mobj->getDocument()->getURL();
    GUrl url = GUrlUtils::rollFileName(msaURL.dirPath() + "/" + msaURL.baseFileName() + ".nwk", DocumentUtils::getNewDocFileNameExcludesHint());
    connect(ui->okButton, SIGNAL(clicked()), SLOT(sl_okClicked()));
    connect(ui->browseButton, SIGNAL(clicked()), SLOT(sl_browseClicked()));
    connect(ui->storeSettings, SIGNAL(clicked()), SLOT(sl_onStoreSettings()));
    connect(ui->restoreSettings, SIGNAL(clicked()), SLOT(sl_onRestoreDefault()));
    
    ui->fileNameEdit->setText(url.getURLString());
    PhyTreeGenerator* generator = registry->getGenerator(nameList.at(0));
    generator->setupCreatePhyTreeUI(this, msa);
    ui->verticalLayout->activate();

    QString algName = AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + "/algorithm", ui->algorithmBox->itemText(0)).toString();
    
    for(int i = 0; i<ui->algorithmBox->count(); i++){
        if(ui->algorithmBox->itemText(i) == algName){
            ui->algorithmBox->setCurrentIndex(i);
            break;
        }
    }
}

void CreatePhyTreeDialogController::sl_okClicked(){
    
    settings.algorithmId = ui->algorithmBox->currentText();
    if (ui->fileNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please, input the file name."));
        ui->fileNameEdit->setFocus();
        return;
    }
    settings.fileUrl = ui->fileNameEdit->text();


    foreach (CreatePhyTreeWidget* widget, childWidgets) {
        widget->fillSettings(settings);
    }

    if(settings.bootstrap){
        if(!checkSeed(settings.seed)){
            QMessageBox::warning(this, tr("Warning"), tr("The seed must be odd."));
            return;
        }
    }

    qint64 memRequiredMB = 0;

    if(estimateResources(&memRequiredMB)){
        QMessageBox mb(QMessageBox::Warning, tr("Warning"), 
            tr("Probably, for that alignment there is no enough memory to run PHYLIP dnadist module. \
            The module will require more than %1 MB in the estimation. \
            \nIt could cause an error. Do you want to continue?").arg(memRequiredMB), QMessageBox::Ok|QMessageBox::Cancel);
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

void CreatePhyTreeDialogController::sl_browseClicked()
{
    GUrl oldUrl = ui->fileNameEdit->text(); 
    QString path;
    LastOpenDirHelper lod;
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

CreatePhyTreeDialogController::~CreatePhyTreeDialogController()
{
    delete ui;
}

bool CreatePhyTreeDialogController::estimateResources(qint64* memoryRequiredMB){
    qint64 appMemMb = 0;
    qint64 minMemoryForDistanceMatrixMb = 0;
    AppResourcePool* s = AppContext::getAppSettings()->getAppResourcePool();
    //AppResourcePool::getCurrentAppMemory(appMemMb);

    appMemMb = s->getMaxMemorySizeInMB();

    //****description******
    //dnadist_makevalues()
//     for (i = 0; i < spp; i++) {
//         nodep[i]->x = (phenotype)Malloc(endsite*sizeof(ratelike));
//         for (j = 0; j < endsite; j++)
//             nodep[i]->x[j]  = (ratelike)Malloc(rcategs*sizeof(sitelike));
//     }

    //rcategs = 1
    //sizeof(sitelike) = 32
    //sizeof(ratelike) = 4
    
    qint64 spp = msa.getNumRows();
    qint64 endsite = msa.getLength();

    qint64 ugeneLowestMemoryUsageMb = 50; 

    minMemoryForDistanceMatrixMb = (qint64)(spp*endsite*32 + endsite*4)/(1024*1024);

    *memoryRequiredMB = minMemoryForDistanceMatrixMb;

    if(minMemoryForDistanceMatrixMb>appMemMb - ugeneLowestMemoryUsageMb){
        return true;
    }else{
        return false;
    }
}

#define SEED_MIN 0
#define SEED_MAX 32767
bool CreatePhyTreeDialogController::checkSeed(int seed){
    return (seed > SEED_MIN) && (seed <SEED_MAX) && (seed%2 == 1);
}

void CreatePhyTreeDialogController::sl_onStoreSettings(){
    AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + "/algorithm", ui->algorithmBox->currentText());

    foreach (CreatePhyTreeWidget* widget, childWidgets) {
        widget->storeSettings();
    }
}
void CreatePhyTreeDialogController::sl_onRestoreDefault(){
    AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + "/algorithm");
    ui->algorithmBox->setCurrentIndex(0);

    foreach (CreatePhyTreeWidget* widget, childWidgets) {
        widget->restoreDefault();
    }
}

}
