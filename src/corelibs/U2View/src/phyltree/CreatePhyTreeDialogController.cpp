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
#include <U2Misc/DialogUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
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
    
    ui->fileNameEdit->setText(url.getURLString());
    PhyTreeGenerator* generator = registry->getGenerator(nameList.at(0));
    generator->setupCreatePhyTreeUI(this, msa);
    ui->verticalLayout->activate();
}

void CreatePhyTreeDialogController::sl_okClicked(){
    
    settings.algorithmId = ui->algorithmBox->currentText();
    if (ui->fileNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("File name is empty"));
        ui->fileNameEdit->setFocus();
        return;
    }
    settings.fileUrl = ui->fileNameEdit->text();

    foreach (CreatePhyTreeWidget* widget, childWidgets) {
        widget->fillSettings(settings);
    }

    if(estimateResources()){
        QMessageBox mb(QMessageBox::Warning, tr("Memory error risk"), tr("For that alignment phylip dnadistance modul will require more memory than you have in system. It may cause crash"));
        mb.exec();
    }

    QDialog::accept();

}

void CreatePhyTreeDialogController::insertWidget( int pos, CreatePhyTreeWidget* widget )
{
    ui->verticalLayout->insertWidget(pos,widget);
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

bool CreatePhyTreeDialogController::estimateResources(){
    int appMemMb = 0;
    int minMemoryForDistanceMatrixMb = 0;
    AppResourcePool* s = AppContext::getAppSettings()->getAppResourcePool();
    //AppResourcePool::getCurrentAppMemory(appMemMb);

    appMemMb = s->getMaxMemorySizeInMB();

    //dnadist_makevalues()
//     for (i = 0; i < spp; i++) {
//         nodep[i]->x = (phenotype)Malloc(endsite*sizeof(ratelike));
//         for (j = 0; j < endsite; j++)
//             nodep[i]->x[j]  = (ratelike)Malloc(rcategs*sizeof(sitelike));
//     }

    //rcategs = 1
    //sizeof(sitelike) = 32
    //sizeof(ratelike) = 4
    
    int spp = msa.getNumRows();
    int endsite = msa.getLength();

    int ugeneLowestMemoryUsedMb = 50; 

    minMemoryForDistanceMatrixMb = (int)(spp*endsite*32 + endsite*4)/(1024*1024);

    if(minMemoryForDistanceMatrixMb>appMemMb - ugeneLowestMemoryUsedMb){
        return true;
    }else{
        return false;
    }

}

}
