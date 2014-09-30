/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/U2FileDialog.h>

#include "ExportCoverageDialog.h"

namespace U2 {

const QString ExportCoverageDialog::DIR_HELPER_NAME = "export_assembly_coverage";

ExportCoverageDialog::ExportCoverageDialog(const QString &assemblyName, QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    initLayout();
    init(assemblyName);
    connectSignals();
}

ExportCoverageSettings ExportCoverageDialog::getSettings() const {
    ExportCoverageSettings settings;
    settings.url = leFilePath->text();
    settings.compress = chbCompress->isChecked();
    settings.exportCoverage = chbCoverage->isChecked();
    settings.exportBasesCount = chbBasesCount->isChecked();
    settings.threshold = sbThreshold->value();
    return settings;
}

void ExportCoverageDialog::accept() {
    if (leFilePath->text().isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("The Otuput file path is not specified."));
        leFilePath->setFocus();
        return;
    }
    if (!checkPermissions()) {
        QMessageBox::critical(this, tr("Error"), tr("Not enough permissions to write here. Please set another output file."));
        return;
    }
    if (!chbCoverage->isChecked() && !chbBasesCount->isChecked()) {
        QMessageBox::critical(this, tr("Error"), tr("Nothing to export"));
        return;
    }

    LastUsedDirHelper dirHelper(DIR_HELPER_NAME);
    dirHelper.url = leFilePath->text();

    QDialog::accept();
}

void ExportCoverageDialog::sl_browseFiles() {
    QFileDialog::Options additionalOptions = 0;
    Q_UNUSED(additionalOptions);
#ifdef Q_OS_MAC
    if (qgetenv("UGENE_GUI_TEST").toInt() == 1 && qgetenv("UGENE_USE_NATIVE_DIALOGS").toInt() == 0) {
        additionalOptions = QFileDialog::DontUseNativeDialog;
    }
#endif
    const QString filter = DialogUtils::prepareDocumentsFileFilter(BaseDocumentFormats::PLAIN_TEXT, true, QStringList());
    LastUsedDirHelper dirHelper(DIR_HELPER_NAME);
    QString filePath = U2FileDialog::getSaveFileName(this,
                                                     tr("Export to..."),
                                                     dirHelper.url,
                                                     filter,
                                                     NULL,
                                                     additionalOptions);
    CHECK(!filePath.isEmpty(), );

    if (chbCompress->isChecked() && !filePath.endsWith(".gz")) {
        filePath += ".gz";
    }

    dirHelper.url = filePath;
    leFilePath->setText(QFileInfo(filePath).absoluteFilePath());
}

void ExportCoverageDialog::sl_compressToggled(bool isChecked) {
    QString filePath = leFilePath->text();
    if (isChecked && !filePath.endsWith(".gz")) {
        leFilePath->setText(filePath + ".gz");
    }
    if (!isChecked && filePath.endsWith(".gz")) {
        filePath.chop(QString(".gz").size());
        leFilePath->setText(filePath);
    }
}

void ExportCoverageDialog::initLayout() {
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Export"));
    new HelpButton(this, buttonBox, "4227548");
}

void ExportCoverageDialog::init(QString assemblyName) {
    LastUsedDirHelper dirHelper(DIR_HELPER_NAME, GUrlUtils::getDefaultDataPath());
    assemblyName.replace(QRegExp("[^0-9a-zA-Z._\\-]"), "_").replace(QRegExp("_+"), "_");
    QString filePath = dirHelper.dir + QDir::separator() + assemblyName + "_coverage.txt" + (chbCompress->isChecked() ? ".gz" : "");
    filePath = GUrlUtils::rollFileName(filePath, "_", QSet<QString>());
    leFilePath->setText(filePath);
}

void ExportCoverageDialog::connectSignals() {
    connect(tbFilePath, SIGNAL(clicked()), SLOT(sl_browseFiles()));
    connect(chbCompress, SIGNAL(toggled(bool)), SLOT(sl_compressToggled(bool)));
}

bool ExportCoverageDialog::checkPermissions() const {
    QFileInfo fileInfo(leFilePath->text());
    QFileInfo dirInfo(fileInfo.absoluteDir().absolutePath());
    bool isFileExist = fileInfo.exists();
    bool isFileWritable = fileInfo.isWritable();
    bool isDirWritable = dirInfo.isWritable();
    while (!dirInfo.exists()) {
        dirInfo = QFileInfo(dirInfo.dir().absolutePath());
        isDirWritable = dirInfo.isWritable();
    }
    return (isFileExist && isFileWritable) || isDirWritable;
}

}   // namespace U2
