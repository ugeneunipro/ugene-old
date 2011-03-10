#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>


#include "GetSequenceByIdDialog.h"
#include <U2Misc/DialogUtils.h>
#include <QtGui/QFileDialog>

namespace U2 {

GetSequenceByIdDialog::GetSequenceByIdDialog(QWidget *w): QDialog(w) {
    setupUi(this);
    connect(toolButton, SIGNAL(clicked()), SLOT(sl_saveFilenameButtonClicked()));
    QString defaultPath = AppContext::getAppSettings()->getUserAppsSettings()->getDownloadDirPath();
    directoryEdit->setText(defaultPath);
    dir = defaultPath;
}

const QString DOWNLOAD_REMOTE_FILE_DOMAIN = "DownloadRemoteFileDialog";

void GetSequenceByIdDialog::sl_saveFilenameButtonClicked() {
    LastOpenDirHelper lod(DOWNLOAD_REMOTE_FILE_DOMAIN);
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Select directory to save"), lod.dir);
    if(!dirName.isEmpty()) {
        directoryEdit->setText(dirName);
        dir = dirName;
    }
}

void GetSequenceByIdDialog::accept() {
    if(dir.isEmpty()) {
        return;
    }

    addToProject = addBox->isChecked();
    QDialog::accept();
}

}