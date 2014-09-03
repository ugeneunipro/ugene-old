
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QMessageBox>
#endif

#include <U2Core/GObjectTypes.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/U2FileDialog.h>

#include "ExpertDiscoveryControlMrkDialog.h"

namespace U2 {

ExpertDiscoveryControlMrkDialog::ExpertDiscoveryControlMrkDialog(QWidget *parent)
: QDialog(parent){

    setupUi(this);
    new HelpButton(this, buttonBox, "7668033");

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
    lod.url = U2FileDialog::getOpenFileName(NULL, tr("Open control sequences markup file"), lod.dir, filter);

    Q_ASSERT(firstFileEdit);
    if (!lod.url.isEmpty()) {
        firstFileEdit->setText(lod.url);
    }
}

}//namespace


