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

#include "MSAExportConsensusTab.h"

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>
#include <U2View/MSAColorScheme.h>
#include <U2View/MSAEditorSequenceArea.h>
#include <U2View/MSAEditorTasks.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/GObjectTypes.h>

namespace U2 {

static const int ITEMS_SPACING = 6;
static const int TITLE_SPACING = 1;

QWidget* MSAExportConsensusTab::createPathGroup(){
    foreach(DocumentFormatId dfid, dfr->getRegisteredFormats()){
        DocumentFormat *df = dfr->getFormatById(dfid);
        foreach (GObjectType type, df->getSupportedObjectTypes()){
            if ((type == GObjectTypes::TEXT || type == GObjectTypes::SEQUENCE) && df->checkFlags(DocumentFormatFlag_SupportWriting)){
                formatCb->addItem(df->getFormatName(), dfid);
            }
        }
    }
    pathLe->setText(AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath() +
        QDir::separator() + "consensus");
    return savePathGroup;
}

MSAExportConsensusTab::MSAExportConsensusTab(MSAEditor* msa_)
:msa(msa_){
    setupUi(this);
    dfr = AppContext::getDocumentFormatRegistry();
    savePath = new ShowHideSubgroupWidget("PATH", tr("Save path"), createPathGroup(), true);
    settings = new ShowHideSubgroupWidget("SETTINGS", tr("Settings"), settingsGroup, true);
    savePathLayout->addWidget(savePath);
    settingsLayout->addWidget(settings);
    skipRb->setChecked(true);

    sl_formatChanged();
    
    connect(browseBtn, SIGNAL(clicked()), SLOT(sl_browseClicked()));
    connect(exportBtn, SIGNAL(clicked()), SLOT(sl_exportClicked()));
    connect(formatCb, SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_formatChanged()));
};

void MSAExportConsensusTab::sl_browseClicked(){
    LastUsedDirHelper h;
    DocumentFormatId id = formatCb->itemData(formatCb->currentIndex()).toString();
    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save file"), h.dir, DialogUtils::prepareDocumentsFileFilter(id, false));
    if (!fileName.isEmpty()) {
        pathLe->setText(fileName);
    }
}

void MSAExportConsensusTab::sl_exportClicked(){
    ExportMSAConsensusTaskSettings settings;
    settings.format = formatCb->itemData(formatCb->currentIndex()).toString();
    settings.addToProjectFlag = addToProjectChb->isChecked();
    settings.keepGaps = keepGapsChb->isChecked();
    settings.msa = msa;
    settings.name = msa->getMSAObject()->getGObjectName() + "_consensus";
    if(skipRb->isChecked()){
        settings.policy = Skip;
    }else if (gapRb->isChecked()){
        settings.policy = ReplaceWithGap;
    }else if(defaultSymbolRb->isChecked()){
        settings.policy = ReplaceWithDefault;
    }else if(keepAllSymbolsRb->isChecked()){
        settings.policy = AllowAllSymbols;
    }
    settings.url = pathLe->text();
    AppContext::getTaskScheduler()->registerTopLevelTask(new ExportMSAConsensusTask(settings));
}

void MSAExportConsensusTab::sl_formatChanged(){
    if (pathLe->text().isEmpty()) {
        return;
    }

    DocumentFormatId id = formatCb->itemData(formatCb->currentIndex()).toString();
    DocumentFormat *df = dfr->getFormatById(id);
    SAFE_POINT(df, "Cant get document format by id", );
    QString fileExt = df->getSupportedDocumentFileExtensions().first();
    GUrl url =  pathLe->text();
    pathLe->setText(QString("%1/%2.%3").arg(url.dirPath()).arg(url.baseFileName()).arg(fileExt));

    bool textSupported = df->getSupportedObjectTypes().contains(GObjectTypes::TEXT);
    keepAllSymbolsRb->setEnabled(textSupported);

    if(keepAllSymbolsRb->isChecked() && !textSupported){
        keepAllSymbolsRb->setChecked(false);
        skipRb->setChecked(true);
    }
}

}
