/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <U2Misc/DialogUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include "BAMDbiPlugin.h"
#include "ConvertToSQLiteDialog.h"

namespace U2 {
namespace BAM {

ConvertToSQLiteDialog::ConvertToSQLiteDialog(bool hasProject) : QDialog(QApplication::activeWindow()) {
    ui.setupUi(this);
    
    if(!hasProject) {
        ui.addToProjectBox->setChecked(false);
        ui.addToProjectBox->setVisible(false);
    }
    setMaximumHeight(layout()->minimumSize().height());
}

const GUrl &ConvertToSQLiteDialog::getSourceUrl()const {
    return sourceUrl;
}

const GUrl &ConvertToSQLiteDialog::getDestinationUrl()const {
    return destinationUrl;
}

bool ConvertToSQLiteDialog::addToProject() const {
    return ui.addToProjectBox->isChecked();
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

static const QString DIR_HELPER_DOMAIN("ConvertToSQLiteDialog");
void U2::BAM::ConvertToSQLiteDialog::on_sourceUrlButton_clicked() {
    LastOpenDirHelper helper(DIR_HELPER_DOMAIN);
    helper.url = QFileDialog::getOpenFileName(this, BAMDbiPlugin::tr("Source BAM File"), helper.dir, 
        BAMDbiPlugin::tr("BAM Files (*.bam);;All Files (*)"));
    if(!helper.url.isEmpty()) {
        ui.sourceUrlEdit->setText(helper.url);
        QFileInfo fi(helper.url);
        QString destCandidate = fi.absoluteDir().path() + "/" + fi.completeBaseName() + ".ugenedb";
        if(QFileInfo(destCandidate).exists()) {
            destCandidate = GUrlUtils::rollFileName(destCandidate, DocumentUtils::getNewDocFileNameExcludesHint());
        }
        ui.destinationUrlEdit->setText(destCandidate);
    }
}

void U2::BAM::ConvertToSQLiteDialog::on_destinationUrlButton_clicked() {
    GUrl url(ui.sourceUrlEdit->text());
    QString dir = url.dirPath() + "/" + url.baseFileName();
    QString returnedValue = QFileDialog::getSaveFileName(this, BAMDbiPlugin::tr("Destination SQLite File"), dir, BAMDbiPlugin::tr("SQLite Files (*.ugenedb);;All Files (*)"));
    if(!returnedValue.isEmpty()) {
        ui.destinationUrlEdit->setText(returnedValue);
    }
}

} // namespace BAM
} // namespace U2
