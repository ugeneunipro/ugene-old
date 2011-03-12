#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include "BAMDbiPlugin.h"
#include "ConvertToSQLiteDialog.h"

namespace U2 {
namespace BAM {

ConvertToSQLiteDialog::ConvertToSQLiteDialog():
    QDialog(NULL)
{
    ui.setupUi(this);
}

const GUrl &ConvertToSQLiteDialog::getSourceUrl()const {
    return sourceUrl;
}
const GUrl &ConvertToSQLiteDialog::getDestinationUrl()const {
    return destinationUrl;
}

void ConvertToSQLiteDialog::accept() {
    sourceUrl = GUrl(ui.sourceUrlEdit->text());
    destinationUrl = GUrl(ui.destinationUrlEdit->text());
    if(sourceUrl.isEmpty()) {
        ui.sourceUrlEdit->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(this, windowTitle(), BAMDbiPlugin::tr("Source URL is not specified"));
    } else if(destinationUrl.isEmpty()) {
        ui.destinationUrlEdit->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(this, windowTitle(), BAMDbiPlugin::tr("Destinaiton URL is not specified"));
    } else if(!destinationUrl.isLocalFile()) {
        ui.destinationUrlEdit->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(this, windowTitle(), BAMDbiPlugin::tr("Destinaiton URL must point to a local file"));
    } else {
        QDialog::accept();
    }
}

void U2::BAM::ConvertToSQLiteDialog::on_sourceUrlButton_clicked() {
    QString returnedValue = QFileDialog::getOpenFileName(this, BAMDbiPlugin::tr("Source BAM File"), QString(), BAMDbiPlugin::tr("BAM Files (*.bam);;All Files (*)"));
    if(!returnedValue.isEmpty()) {
        ui.sourceUrlEdit->setText(returnedValue);
    }
}

void U2::BAM::ConvertToSQLiteDialog::on_destinationUrlButton_clicked() {
    GUrl url(ui.sourceUrlEdit->text());
    QString dir = url.dirPath() + "/" + url.baseFileName();
    QString returnedValue = QFileDialog::getSaveFileName(this, BAMDbiPlugin::tr("Destination SQLite File"), dir, BAMDbiPlugin::tr("SQLite Files (*.sqlite);;All Files (*)"));
    if(!returnedValue.isEmpty()) {
        ui.destinationUrlEdit->setText(returnedValue);
    }
}

} // namespace BAM
} // namespace U2
