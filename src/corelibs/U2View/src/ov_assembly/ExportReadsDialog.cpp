/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/U2FileDialog.h>

#include "ExportReadsDialog.h"

namespace U2 {

ExportReadsDialog::ExportReadsDialog(QWidget * p, const QList<DocumentFormatId> & formats) : QDialog(p) {
    setupUi(this);
    new HelpButton(this, buttonBox, "4227544");

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Export"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    documentFormatComboBox->addItems(formats);
    initFilePath();

    connect(filepathToolButton, SIGNAL(clicked()), SLOT(sl_selectFile()));
    connect(documentFormatComboBox, SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_formatChanged(const QString &)));
    setMaximumHeight(layout()->minimumSize().height());
}

void ExportReadsDialog::accept() {
    if(filepathLineEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Error!"), tr("Select destination file"));
        filepathLineEdit->setFocus(Qt::OtherFocusReason);
        return;
    }
    QDialog::accept();
}

void ExportReadsDialog::sl_selectFile() {
    LastUsedDirHelper lod("ExportReadsDialog");
    const QString filter = DialogUtils::prepareDocumentsFileFilter(documentFormatComboBox->currentText(), false, QStringList());
    lod.url = U2FileDialog::getSaveFileName(this, tr("Select file to save"), lod, filter);
    if (lod.url.isEmpty()) {
        return;
    }
    filepathLineEdit->setText(lod.url);
}

void ExportReadsDialog::sl_formatChanged(const QString &newFormat) {
    filepathLineEdit->setText(GUrlUtils::rollFileName(GUrlUtils::changeFileExt(filepathLineEdit->text(), newFormat).getURLString(), "_"));
}

void ExportReadsDialog::initFilePath() {
    const QString ugeneDataDir = AppContext::getAppSettings()->getUserAppsSettings()->getDefaultDataDirPath();
    LastUsedDirHelper lod("ExportReadsDialog", ugeneDataDir);

    DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(documentFormatComboBox->currentText());
    CHECK(NULL != format, );
    const QStringList extensions = format->getSupportedDocumentFileExtensions();
    CHECK(!extensions.isEmpty(), );

    const QString filePath = lod.dir + QDir::separator() + "exported_reads" + "." + extensions.first();
    filepathLineEdit->setText(GUrlUtils::rollFileName(filePath, "_", QSet<QString>()));
}

ExportReadsDialogModel ExportReadsDialog::getModel() const {
    ExportReadsDialogModel ret;
    ret.filepath = filepathLineEdit->text();
    ret.format = documentFormatComboBox->currentText();
    ret.addToProject = addToProjectCheckBox->isChecked();
    return ret;
}

} // U2
