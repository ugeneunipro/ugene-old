#include "ExpertDiscoveryControlMrkDialog.h"

#include <U2Core/GObjectTypes.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#endif
#include <U2Gui/HelpButton.h>


namespace U2 {

ExpertDiscoveryControlMrkDialog::ExpertDiscoveryControlMrkDialog(QWidget *parent)
: QDialog(parent){

    setupUi(this);
    new HelpButton(this, buttonBox, "4227131");

    connect(openFirstButton, SIGNAL(clicked()), SLOT(sl_openFirstFile()));

    filter = DialogUtils::prepareFileFilter("Markup files", QStringList() << "xml" << "gb");

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
    LastUsedDirHelper lod("ExpertDiscovery control sequences markup file");
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open control sequences markup file"), lod.dir, filter);

    Q_ASSERT(firstFileEdit);
    if (!lod.url.isEmpty()) {
        firstFileEdit->setText(lod.url);
    }
}

}//namespace


