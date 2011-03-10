#include "ExportAnnotations2CSVDialog.h"
#include "ui_ExportAnnotations2CSVDialog.h"

#include <QtGui/QFileDialog>

namespace U2 {

ExportAnnotations2CSVDialog::ExportAnnotations2CSVDialog(QWidget *parent) 
: QDialog(parent), m_ui(new Ui::ExportAnnotations2CSVDialog())
{
    m_ui->setupUi(this);
}

ExportAnnotations2CSVDialog::~ExportAnnotations2CSVDialog() {
    delete m_ui;
}

QString ExportAnnotations2CSVDialog::getFileName() const {
    return m_ui->fileNameEdit->text();
}

void ExportAnnotations2CSVDialog::setFileName(const QString& value) {
    m_ui->fileNameEdit->setText(value);
}

bool ExportAnnotations2CSVDialog::getExportSequence()const {
    return m_ui->exportSequenceCheck->isChecked();
}

void ExportAnnotations2CSVDialog::setExportSequence(bool value) {
    m_ui->exportSequenceCheck->setChecked(value);
}

void ExportAnnotations2CSVDialog::changeEvent(QEvent *e) {
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            m_ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void ExportAnnotations2CSVDialog::on_chooseFileButton_clicked() {
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this, QString(), QString(), tr("CSV files (*.csv);;All files (*)"));
    if(!fileName.isEmpty()) {
        m_ui->fileNameEdit->setText(fileName);
    }
}

void ExportAnnotations2CSVDialog::setExportSequenceEnabled( bool value )
{   
    m_ui->exportSequenceCheck->setEnabled(value);
}

} // namespace U2
