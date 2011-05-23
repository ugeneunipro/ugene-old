#include "ExpertDiscoveryControlMrkDialog.h"

#include <U2Core/GObjectTypes.h>
#include <U2Misc/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2 {

ExpertDiscoveryControlMrkDialog::ExpertDiscoveryControlMrkDialog(QWidget *parent)
: QDialog(parent){

    setupUi(this);

    connect(openFirstButton, SIGNAL(clicked()), SLOT(sl_openFirstFile()));

    filter = DialogUtils::prepareFileFilter("Markup files", QStringList() << "xml");
}

void ExpertDiscoveryControlMrkDialog::accept(){
    Q_ASSERT(firstFileEdit);

    firstFileName = firstFileEdit->text();


    if (!firstFileName.isEmpty()) {
        QDialog::accept();
    }else {
        QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Select files for ExpertDiscovery"));
        mb.exec();
    }
}

void ExpertDiscoveryControlMrkDialog::sl_openFirstFile(){
    LastOpenDirHelper lod("ExpertDiscovery control sequences markup file");
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open control sequences markup file"), lod.dir, filter);

    Q_ASSERT(firstFileEdit);
    if (!lod.url.isEmpty()) {
        firstFileEdit->setText(lod.url);
    }
}

}//namespace


