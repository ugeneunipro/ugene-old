#include "ExportProjectDialogController.h"

#include <U2Misc/DialogUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>

#include <QtGui/qfiledialog.h>
#include <QtGui/qmessagebox.h>

namespace U2{

static QString fixProjectFile(const QString& name) {
    QString result = name;
    if (result.isEmpty()) {
        result =  "project" + PROJECTFILE_EXT;
    } else if (!result.endsWith(PROJECTFILE_EXT)) {
        result+=PROJECTFILE_EXT;
    }
    return result;
}

ExportProjectDialogController::ExportProjectDialogController(QWidget *p, const QString& defaultProjectFileName) 
: QDialog(p) 
{
	setupUi(this);
    setModal(true);
    projectFile = fixProjectFile(defaultProjectFileName);
    projectFileEdit->setText(projectFile);
	connect(browseButton, SIGNAL(clicked()), this, SLOT(sl_onBrowseButton()));
}

void ExportProjectDialogController::accept(){
    QString dirPath = exportFolderEdit->text();
    projectFile = fixProjectFile(projectFileEdit->text());
    
    QString error;
    exportDir = GUrlUtils::prepareDirLocation(dirPath, error);
    if (exportDir.isEmpty()) {
        assert(!error.isEmpty());
        QMessageBox::critical(this, this->windowTitle(), error);
        return;
    }
	QDialog::accept();
}

void ExportProjectDialogController::sl_onBrowseButton(){
	LastOpenDirHelper h;
	QString folder = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), h.dir);
    if (folder.isEmpty()) {
        return;
    }
	exportFolderEdit->setText(folder);
}

}
