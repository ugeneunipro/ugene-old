/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QPushButton>
#include <QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/HelpButton.h>
#include <U2Gui/SaveDocumentController.h>

#include "ExportHighlightedDialogController.h"
#include "ov_msa/MSAEditorSequenceArea.h"

namespace U2{

ExportHighligtingDialogController::ExportHighligtingDialogController(MSAEditorUI *msaui_, QWidget* p )
    : QDialog(p),
      msaui(msaui_),
      saveController(NULL)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "17467629");

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Export"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    CHECK(AppContext::getAppSettings(), );
    CHECK(AppContext::getAppSettings()->getUserAppsSettings(), );
    CHECK(msaui->getEditor(), );
    CHECK(msaui->getEditor()->getMSAObject(), );

    initSaveController();

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

void ExportHighligtingDialogController::accept(){
    startPos = startPosBox->value();
    endPos = endPosBox->value();
    if(oneIndexRB->isChecked()){
        startingIndex = 1;
    }else{
        startingIndex = 0;
    }
    if (saveController->getSaveFileName().isEmpty()){
        QMessageBox::warning(this, tr("Warning"), tr("Export to file URL is empty!"));
        return;
    }
    keepGaps = keepGapsBox->isChecked();
    dots = dotsBox->isChecked();
    transpose = transposeBox->isChecked();
    url = GUrl(saveController->getSaveFileName());

    QDialog::accept();
}

void ExportHighligtingDialogController::lockKeepGaps(){
    keepGapsBox->setChecked(true);
    keepGapsBox->setDisabled(true);
}

void ExportHighligtingDialogController::endPosValueChanged(){
    startPosBox->setMaximum(endPosBox->value() - 1);
}

void ExportHighligtingDialogController::initSaveController() {
    SaveDocumentControllerConfig config;
    config.defaultFileName = GUrlUtils::getDefaultDataPath() + "/" + msaui->getEditor()->getMSAObject()->getGObjectName() + "_highlighting.txt";
    config.defaultFormatId = BaseDocumentFormats::PLAIN_TEXT;
    config.fileDialogButton = fileButton;
    config.fileNameEdit = fileNameEdit;
    config.parentWidget = this;
    config.saveTitle = tr("Select file to save...");

    const QList<DocumentFormatId> formats = QList<DocumentFormatId>() << BaseDocumentFormats::PLAIN_TEXT;

    saveController = new SaveDocumentController(config, formats, this);
}

}
