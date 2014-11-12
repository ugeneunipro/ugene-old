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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QPushButton>
#endif

#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/U2FileDialog.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>

#include "ExportHighlightedDialogController.h"
#include "ov_msa/MSAEditorSequenceArea.h"

namespace U2{

ExportHighligtningDialogController::ExportHighligtningDialogController(MSAEditorUI *msaui_, QWidget* p ): QDialog(p), msaui(msaui_){
    setupUi(this);
    new HelpButton(this, buttonBox, "4227438");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Export"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    CHECK(AppContext::getAppSettings(), );
    CHECK(AppContext::getAppSettings()->getUserAppsSettings(), );
    CHECK(msaui->getEditor(), );
    CHECK(msaui->getEditor()->getMSAObject(), );    
    fileNameEdit->setText(QDir::toNativeSeparators(AppContext::getAppSettings()->getUserAppsSettings()->getDefaultDataDirPath() + "/" + msaui->getEditor()->getMSAObject()->getGObjectName() + "_highlighting.txt"));

    connect(fileButton, SIGNAL(clicked()), SLOT(sl_fileButtonClicked()));
    connect(endPosBox, SIGNAL(valueChanged(int)), SLOT(endPosValueChanged()));

    int alignLength = msaui->getEditor()->getMSAObject()->getLength();
    QRect selection = msaui->getSequenceArea()->getSelection().getRect();

    int startPos = -1;
    int endPos = -1;
    if (selection.isNull()) {
        startPos = 1;
        endPos = alignLength;
    } else {
        startPos = selection.x() + 1;
        endPos = selection.x() + selection.width();
    }

    startPosBox->setMaximum(endPos);
    endPosBox->setMaximum(alignLength);
    
    startPosBox->setMinimum(1);
    endPosBox->setMinimum(2);

    startPosBox->setValue(startPos);
    endPosBox->setValue(endPos);

}

void ExportHighligtningDialogController::accept(){
    startPos = startPosBox->value();
    endPos = endPosBox->value();
    if(oneIndexRB->isChecked()){
        startingIndex = 1;
    }else{
        startingIndex = 0;
    }
    keepGaps = keepGapsBox->isChecked();
    dots = dotsBox->isChecked();
    url = GUrl(fileNameEdit->text());
    
    QDialog::accept();
}

void ExportHighligtningDialogController::sl_fileButtonClicked(){
    LastUsedDirHelper h;

    h.url = U2FileDialog::getSaveFileName(this, tr("Select file to save..."), h.dir);
    fileNameEdit->setText(h.url);
}

void ExportHighligtningDialogController::lockKeepGaps(){
    keepGapsBox->setChecked(true);
    keepGapsBox->setDisabled(true);
}

void ExportHighligtningDialogController::endPosValueChanged(){
    startPosBox->setMaximum(endPosBox->value() - 1);
}

}
