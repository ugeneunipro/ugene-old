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

#include <QMessageBox>
#include <QPushButton>

#include <U2Core/GObjectTypes.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Core/QObjectScopedPointer.h>
#include <U2Gui/U2FileDialog.h>

#include "ExpertDiscoveryPosNegDialog.h"

namespace U2 {

ExpertDiscoveryPosNegDialog::ExpertDiscoveryPosNegDialog(QWidget *parent)
: QDialog(parent), generateNeg(false), negativePerPositive(100){

    setupUi(this);
    new HelpButton(this, buttonBox, "16122413");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Next"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connect(openFirstButton, SIGNAL(clicked()), SLOT(sl_openFirstFile()));
    connect(openSecondButton, SIGNAL(clicked()), SLOT(sl_openSecondFile()));
    connect(oneSequenceCheckBox, SIGNAL(clicked()), SLOT(sl_oneSequence()));

    filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true)+
        ";;" + DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, false);

    negPerPositivespinBox->setDisabled(!oneSequenceCheckBox->isChecked());

}

void ExpertDiscoveryPosNegDialog::accept(){
    Q_ASSERT(firstFileEdit);
    Q_ASSERT(secondFileEdit);

    firstFileName = firstFileEdit->text();
    secondFileName = secondFileEdit->text();
    negativePerPositive =  negPerPositivespinBox->value();

    if (oneSequenceCheckBox->isChecked()) {
        secondFileName = firstFileName;
    }

    if (!firstFileName.isEmpty() && !secondFileName.isEmpty()) {
        if(secondFileName == firstFileName && !generateNeg){
            QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Select files"), tr("Positive and negative sequences can't be in the one file. Select another file for one of them"));
            mb->exec();
            CHECK(!mb.isNull(), );
        }else{
            QDialog::accept();
        }
    }else {
        QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Select files"), tr("Select files for ExpertDiscovery"));
        mb->exec();
        CHECK(!mb.isNull(), );
    }
}

void ExpertDiscoveryPosNegDialog::sl_oneSequence() {

    secondFileEdit->setDisabled(oneSequenceCheckBox->isChecked());
    openSecondButton->setDisabled(oneSequenceCheckBox->isChecked());
    negPerPositivespinBox->setDisabled(!oneSequenceCheckBox->isChecked());
    generateNeg = oneSequenceCheckBox->isChecked();

}

void ExpertDiscoveryPosNegDialog::sl_openFirstFile(){
    LastUsedDirHelper lod("ExpertDiscovery positive sequences file");
    lod.url = U2FileDialog::getOpenFileName(NULL, tr("Open positive sequences file"), lod.dir, filter);

    Q_ASSERT(firstFileEdit);
    if (!lod.url.isEmpty()) {
        firstFileEdit->setText(lod.url);
    }
}
void ExpertDiscoveryPosNegDialog::sl_openSecondFile() {

    LastUsedDirHelper lod("ExpertDiscovery negative sequences file");
    if (lod.dir.isEmpty()) {
        LastUsedDirHelper lodFirst("Open negative sequences file");

        lod.dir = lodFirst.dir;
    }
    lod.url = U2FileDialog::getOpenFileName(NULL, tr("Open second file"), lod.dir, filter);

    Q_ASSERT(secondFileEdit);
    if (!lod.url.isEmpty()) {
        secondFileEdit->setText(lod.url);
    }
}

}//namespace


