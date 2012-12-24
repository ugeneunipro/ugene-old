#include "TmpDirChangeDialogController.h"

namespace U2 {

TmpDirChangeDialogController::TmpDirChangeDialogController(QString path, QWidget* p) : QDialog(p), tmpDirPath(path) {
    setupUi(this);
    QString message = "You do not have permission to write to \"" + tmpDirPath +
                         "\" directory. Please, set the valid temp directory:";
    messageText->setText(tr(message.toAscii()));
    tmpDirPathEdit->setText(tmpDirPath);
    tmpDirChecker = new TmpDirChecker;

    connect(browseTmpDirButton, SIGNAL(clicked()), this, SLOT(sl_changeDirButtonClicked()));
    connect(exitAppButton, SIGNAL(clicked()), this, SLOT(sl_exitAppButtonClicked()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(sl_okButtonClicked()));
}

QString TmpDirChangeDialogController::getTmpDirPath() {
    return tmpDirPath;
}

void TmpDirChangeDialogController::sl_changeDirButtonClicked() {
    QString newPath = QFileDialog::getExistingDirectory(parentWidget(), tr("Choose Directory"), tmpDirPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (newPath.isEmpty() == false)
        tmpDirPathEdit->setText(newPath);
}

void TmpDirChangeDialogController::sl_exitAppButtonClicked() {
    QDialog::reject();
}

void TmpDirChangeDialogController::sl_okButtonClicked() {
    tmpDirPath = tmpDirPathEdit->text();
    if (!tmpDirChecker->checkPath(tmpDirPath)) {
        QString message = "You do not have permission to write to \"" + tmpDirPath +
                             "\" directory. Please, set the valid temp directory:";
        messageText->setText(tr(message.toAscii()));
    }
    else {
        QDialog::accept();
    }
}

}// namespace
