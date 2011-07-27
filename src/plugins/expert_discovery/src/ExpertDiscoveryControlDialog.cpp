#include "ExpertDiscoveryControlDialog.h"

#include <U2Core/GObjectTypes.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/LastUsedDirHelper.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2 {

ExpertDiscoveryControlDialog::ExpertDiscoveryControlDialog(QWidget *parent)
: QDialog(parent){

    setupUi(this);

    connect(openFirstButton, SIGNAL(clicked()), SLOT(sl_openFirstFile()));

    filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true);
}

void ExpertDiscoveryControlDialog::accept(){
    Q_ASSERT(firstFileEdit);

    firstFileName = firstFileEdit->text();


    if (!firstFileName.isEmpty()) {
        QDialog::accept();
    }else {
        QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Select files for ExpertDiscovery"));
        mb.exec();
    }
}

void ExpertDiscoveryControlDialog::sl_openFirstFile(){
    LastUsedDirHelper lod("ExpertDiscovery control sequences file");
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open control sequences file"), lod.dir, filter);

    Q_ASSERT(firstFileEdit);
    if (!lod.url.isEmpty()) {
        firstFileEdit->setText(lod.url);
    }
}

}//namespace


