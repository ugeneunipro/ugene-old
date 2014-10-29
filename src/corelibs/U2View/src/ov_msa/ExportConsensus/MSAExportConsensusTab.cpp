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

#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/L10n.h>
#include <U2Core/U2IdTypes.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Gui/U2FileDialog.h>
#include <U2Gui/U2WidgetStateStorage.h>

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorConsensusArea.h>
#include <U2View/MSAEditorSequenceArea.h>
#include <U2View/MSAEditorTasks.h>

#include "MSAExportConsensusTab.h"

namespace U2 {

static const int ITEMS_SPACING = 6;
static const int TITLE_SPACING = 1;

MSAExportConsensusTab::MSAExportConsensusTab(MSAEditor* msa_)
    : msa(msa_), savableWidget(this, GObjectViewUtils::findViewByName(msa_->getName()))
{
    setupUi(this);

    hintLabel->setStyleSheet("color: green; font: bold;");
    
    DocumentFormatRegistry *dfr = AppContext::getDocumentFormatRegistry();
    pathLe->setText(AppContext::getAppSettings()->getUserAppsSettings()->getDefaultDataDirPath() + QDir::separator()
        + msa->getMSAObject()->getGObjectName() + "_consensus.txt");

    formatCb->addItem(dfr->getFormatById(BaseDocumentFormats::PLAIN_TEXT)->getFormatName(), BaseDocumentFormats::PLAIN_TEXT);

    MSAEditorConsensusArea *consensusArea = msa->getUI()->getConsensusArea();
    showHint(true);
    
    sl_consensusChanged(consensusArea->getConsensusAlgorithm()->getId());

    connect(browseBtn, SIGNAL(clicked()), SLOT(sl_browseClicked()));
    connect(exportBtn, SIGNAL(clicked()), SLOT(sl_exportClicked()));
    connect(formatCb, SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_formatChanged()));
    connect(consensusArea, SIGNAL(si_consensusAlgorithmChanged(const QString &)), SLOT(sl_consensusChanged(const QString &)));

    U2WidgetStateStorage::restoreWidgetState(savableWidget);
};

void MSAExportConsensusTab::sl_browseClicked() {
    LastUsedDirHelper h;
    DocumentFormatId id = formatCb->itemData(formatCb->currentIndex()).toString();
    QString fileName = U2FileDialog::getSaveFileName(NULL, tr("Save file"), h.dir, DialogUtils::prepareDocumentsFileFilter(id, false));
    if (!fileName.isEmpty()) {
        pathLe->setText(fileName);
    }
}

void MSAExportConsensusTab::sl_exportClicked(){
    ExportMSAConsensusTaskSettings settings;
    settings.format = formatCb->itemData(formatCb->currentIndex()).toString();
    settings.keepGaps = keepGapsChb->isChecked();
    settings.msa = msa;
    settings.name = msa->getMSAObject()->getGObjectName() + "_consensus";

    settings.url = pathLe->text();
    AppContext::getTaskScheduler()->registerTopLevelTask(new ExportMSAConsensusTask(settings));
}

void MSAExportConsensusTab::sl_formatChanged(){
    if (pathLe->text().isEmpty()) {
        return;
    }

    DocumentFormatId id = formatCb->itemData(formatCb->currentIndex()).toString();
    DocumentFormatRegistry *dfr = AppContext::getDocumentFormatRegistry();
    DocumentFormat *df = dfr->getFormatById(id);
    SAFE_POINT(df, "Cant get document format by id", );
    QString fileExt = df->getSupportedDocumentFileExtensions().first();
    GUrl url =  pathLe->text();
    pathLe->setText(QDir::toNativeSeparators(QString( QString("%1") + QDir::separator() + QString("%2.%3") )
        .arg(url.dirPath()).arg(url.baseFileName()).arg(fileExt)));
}

void MSAExportConsensusTab::showHint( bool showHint ){
    if (showHint){
        hintLabel->show();
        keepGapsChb->hide();
    }else{
        hintLabel->hide();
        keepGapsChb->show();
    }
}

void MSAExportConsensusTab::sl_consensusChanged(const QString& algoId) {
    MSAConsensusAlgorithmFactory *consAlgorithmFactory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(algoId);
    SAFE_POINT(consAlgorithmFactory != NULL, "Fetched consensus algorithm factory is NULL", );
    if(consAlgorithmFactory->isSequenceLikeResult()){
        if(formatCb->count() == 1 ){ //only text
            DocumentFormatRegistry *dfr = AppContext::getDocumentFormatRegistry();
            formatCb->addItem(dfr->getFormatById(BaseDocumentFormats::PLAIN_GENBANK)->getFormatName(), BaseDocumentFormats::PLAIN_GENBANK);
            formatCb->addItem(dfr->getFormatById(BaseDocumentFormats::FASTA)->getFormatName(), BaseDocumentFormats::FASTA);
            showHint(false);
        }else{
            SAFE_POINT(formatCb->count() == 3, "Count of supported 'text' formats is not equal three", );
        }        
    }else{
        if(formatCb->count() == 3 ){ //all possible formats
            formatCb->setCurrentIndex(formatCb->findText(BaseDocumentFormats::PLAIN_TEXT));
            formatCb->removeItem(formatCb->findText(BaseDocumentFormats::FASTA));
            formatCb->removeItem(formatCb->findText(BaseDocumentFormats::PLAIN_GENBANK));
            showHint(true);
        }else{
            SAFE_POINT(formatCb->count() == 1, "Count of supported 'text' formats is not equal one", );
        }
    }
}

}
