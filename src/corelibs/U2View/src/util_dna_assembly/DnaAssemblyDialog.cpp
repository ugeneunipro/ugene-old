/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QKeyEvent>


#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ExternalToolRegistry.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/AppSettingsGUI.h>

#include <U2Algorithm/DnaAssemblyAlgRegistry.h>

#include "DnaAssemblyDialog.h"
#include "DnaAssemblyGUIExtension.h"

namespace U2 {

QStringList DnaAssemblyDialog::lastShortReadsUrls;
QString DnaAssemblyDialog::lastRefSeqUrl;
QString DnaAssemblyDialog::methodName;
bool DnaAssemblyDialog::prebuiltIndex = false;
bool DnaAssemblyDialog::samOutput = false;

#define LIBRARY_TYPE_PAIRED     "Paired-end"
#define LIBRARY_TYPE_SINGLE     "Single-end"
#define MATE_UPSTREAM           "Upstream"
#define MATE_DOWNSTREAM         "Downstream"

DnaAssemblyDialog::DnaAssemblyDialog(QWidget* p, const QStringList& shortReadsUrls, const QString& refSeqUrl)
: QDialog(p), assemblyRegistry(AppContext::getDnaAssemblyAlgRegistry()), customGUI(NULL)
{
    setupUi(this);
    QStringList names = assemblyRegistry->getRegisteredAlgorithmIds();
    methodNamesBox->addItems(names);
    // TODO: change the way default method is set
    if (names.size() > 0) {
        int res = -1;
        if (!methodName.isEmpty()) {
            res = methodNamesBox->findText(methodName);
        }
        if (-1 == res) {
            methodNamesBox->setCurrentIndex(names.size() - 1);
        } else {
            methodNamesBox->setCurrentIndex(res);
        }
    }
    
    shortReadsTable->installEventFilter(this);
    QHeaderView* header = shortReadsTable->header();
    header->setClickable( false );
    header->setStretchLastSection( false );
    header->setResizeMode( 0, QHeaderView::Stretch );

    prebuiltIndexCheckBox->setChecked(prebuiltIndex);
    sl_onAlgorithmChanged(methodNamesBox->currentText());
    sl_onPrebuiltIndexBoxClicked();
    connect(addShortreadsButton, SIGNAL(clicked()), SLOT(sl_onAddShortReadsButtonClicked()) );
    connect(removeShortReadsButton, SIGNAL(clicked()), SLOT(sl_onRemoveShortReadsButtonClicked()));
    connect(setResultFileNameButton, SIGNAL(clicked()), SLOT(sl_onSetResultFileNameButtonClicked()));
    connect(addRefButton, SIGNAL(clicked()), SLOT(sl_onAddRefButtonClicked()) );
    connect(methodNamesBox, SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_onAlgorithmChanged(const QString &)));
    connect(prebuiltIndexCheckBox, SIGNAL(clicked()), SLOT(sl_onPrebuiltIndexBoxClicked()));
    connect(samBox, SIGNAL(clicked()), SLOT(sl_onSamBoxClicked()));
    connect(libraryComboBox, SIGNAL(currentIndexChanged(int)), SLOT(sl_onLibraryTypeChanged()));
    
    QString activeRefSeqUrl = refSeqUrl.isEmpty() ? lastRefSeqUrl : refSeqUrl;
    if (!activeRefSeqUrl.isEmpty()) {
        refSeqEdit->setText(activeRefSeqUrl);
        buildResultUrl(activeRefSeqUrl);
        if (NULL != customGUI) {
            QString error;
            customGUI->buildIndexUrl(lastRefSeqUrl, prebuiltIndex, error);
            //todo: process error!
        }
    }


    //TODO: deal with it later
    /*QStringList activeShortReadsUrl = shortReadsUrls.isEmpty() ? lastShortReadsUrls : shortReadsUrls;
    foreach(const QString& read, activeShortReadsUrl) {
        shortReadsList->addItem(read);
    }*/

	/*

    //TODO Delete this crap after 01.11.2012
	
    //HACK for testing UGENE-1092
	refSeqEdit->setText("W:/chrY-trunc.fa");
	samBox->setChecked(true);
	sl_onSamBoxClicked();
	QFile::remove("W:/chrY-trunc.sam");
	buildResultUrl(GUrl("W:/chrY-trunc.fa"));
    if (NULL != customGUI) {
        QString error;
        if (!customGUI->buildIndexUrl(GUrl("W:/chrY-trunc.fa"), prebuiltIndex, error)) {
            QMessageBox::information(this, "DNA Assembly", error);
        }
    }

	shortReadsList->clear();
	shortReadsList->addItem(new QListWidgetItem(QString("W:\\chrY-trunc-reads.fasta")));
	//KCAH!!
	*/
}

void DnaAssemblyDialog::updateState() {
    addGuiExtension();
}

void DnaAssemblyDialog::sl_onAddShortReadsButtonClicked() {
    LastUsedDirHelper lod;
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add short reads"), lod.dir);
    if (fileNames.isEmpty()) {
        return;
    }
    lod.url = fileNames.at(fileNames.count() - 1);

    QList<QTreeWidgetItem*> items;

    foreach(const QString& f, fileNames) {
        ShortReadsTableItem* item = new ShortReadsTableItem(shortReadsTable, f);
        item->setLibraryType( libraryComboBox->currentIndex() == 0 ? LIBRARY_TYPE_SINGLE : LIBRARY_TYPE_PAIRED);
        ShortReadsTableItem::addItemToTable(item, shortReadsTable);
    }



}

void DnaAssemblyDialog::sl_onAddRefButtonClicked() {
    LastUsedDirHelper lod;
    QString filter;

    lod.url = QFileDialog::getOpenFileName(this, tr("Open reference sequence"), lod.dir, filter);
    if (lod.url.isEmpty()) {
        return;
    }

    refSeqEdit->setText(lod.url);
    buildResultUrl(lod.url);
    if (NULL != customGUI) {
        QString error;
        if (!customGUI->buildIndexUrl(lod.url, prebuiltIndex, error)) {
            QMessageBox::information(this, "DNA Assembly", error);
        }
    }
} 

void DnaAssemblyDialog::accept() {
    if (NULL != customGUI) {
        QString error;
        if (!customGUI->isParametersOk(error)) {
            if (!error.isEmpty()) {
                QMessageBox::information(this, tr("Align short reads"), error);
            }
            return;
        }
        if (!customGUI->isIndexOk(error, refSeqEdit->text())) {
            if (!prebuiltIndex) {
                QMessageBox::StandardButton res = QMessageBox::warning(this, tr("DNA Assembly"), error, QMessageBox::Ok | QMessageBox::Cancel);
                if (QMessageBox::Cancel == res) {
                    return;
                }
            } else {
                QMessageBox::information(this, tr("DNA Assembly"), error);
                return;
            }
        }
    }
    
    if (refSeqEdit->text().isEmpty()) {
        QMessageBox::information(this, tr("DNA Assembly"),
            tr("Reference sequence url is not set!") );
    } else if (resultFileNameEdit->text().isEmpty() ) {
        QMessageBox::information(this, tr("DNA Assembly"),
            tr("Result alignment file name is not set!") );
    } else if (shortReadsTable->topLevelItemCount() == 0 ) {
        QMessageBox::information(this, tr("DNA Assembly"),
            tr("Short reads list is empty!") );
    } else {
        
        lastRefSeqUrl.clear();
        lastShortReadsUrls.clear();

        lastRefSeqUrl = refSeqEdit->text();
        int numItems = shortReadsTable->topLevelItemCount();
        for( int i =0; i < numItems; ++i) {
            lastShortReadsUrls.append(shortReadsTable->topLevelItem(i)->data(0,0).toString());
        }
        
        QDialog::accept();
    }
}

const GUrl DnaAssemblyDialog::getRefSeqUrl() {
    return refSeqEdit->text();
}

const QList<ShortReadSet> DnaAssemblyDialog::getShortReadSets()
{
    QList<ShortReadSet> sets;

    int numItems = shortReadsTable->topLevelItemCount();

    for( int i =0; i < numItems; ++i) {
        ShortReadsTableItem* item = static_cast<ShortReadsTableItem*> ( shortReadsTable->topLevelItem(i) );
        sets.append( ShortReadSet(item->getUrl(), item->getType(), item->getOrder()));

    }
    return sets;

}

const QString DnaAssemblyDialog::getAlgorithmName() {
    return methodNamesBox->currentText();
}

void DnaAssemblyDialog::sl_onRemoveShortReadsButtonClicked() {
    int currentRow = shortReadsTable->currentIndex().row();
    shortReadsTable->takeTopLevelItem(currentRow);
    
}

void DnaAssemblyDialog::sl_onSetResultFileNameButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Set result alignment file name"), lod.dir);
    if (!lod.url.isEmpty()) {
        GUrl result = lod.url;
        buildResultUrl(result);
    }
}

void DnaAssemblyDialog::sl_onAlgorithmChanged(const QString &text) {
    methodName = text;
    updateState();
}

void DnaAssemblyDialog::sl_onPrebuiltIndexBoxClicked() {
    prebuiltIndex = prebuiltIndexCheckBox->isChecked();

    if (customGUI != NULL) {
        customGUI->prebuiltIndex(prebuiltIndex);
        if (refSeqEdit->text().isEmpty()) {
            return;
        }
        QString error;
        if (!customGUI->buildIndexUrl(refSeqEdit->text(), prebuiltIndex, error)) {
            QMessageBox::information(this, "DNA Assembly", error);
        }
    }
}

void DnaAssemblyDialog::sl_onSamBoxClicked() {
    samOutput = samBox->isChecked();

    if (!refSeqEdit->text().isEmpty()) {
        buildResultUrl(refSeqEdit->text());
    }
}

const QString DnaAssemblyDialog::getResultFileName() {
    return resultFileNameEdit->text();
}

bool DnaAssemblyDialog::isPrebuiltIndex() const {
    return prebuiltIndexCheckBox->isChecked();
}

bool DnaAssemblyDialog::isSamOutput() const {
    return samBox->isChecked();
}

QMap<QString, QVariant> DnaAssemblyDialog::getCustomSettings() {
    if (customGUI != NULL) {
        return customGUI->getDnaAssemblyCustomSettings();
    } else {
        return QMap<QString, QVariant>();
    }
}

void DnaAssemblyDialog::addGuiExtension() {
    static const int insertPos = verticalLayout->count() - 2;

    int macFixDelta = 50;

    // cleanup previous extension
    if (customGUI != NULL) {
        layout()->removeWidget(customGUI);         
        setMinimumHeight(minimumHeight() - customGUI->minimumHeight());
        delete customGUI;
        customGUI = NULL;
        macFixDelta = 0;
    }
    
    // insert new extension widget
    DnaAssemblyAlgorithmEnv* env = assemblyRegistry->getAlgorithm(methodNamesBox->currentText());
    
    if (!env->supportsPairedEndLibrary()) {
        libraryComboBox->setEnabled(false);
        libraryComboBox->setCurrentIndex(0);
    } else {
        libraryComboBox->setEnabled(true);
    }
    
    if (NULL == env) {
        adjustSize();
        return;
    }
    if (!env->isDbiSupported()) {
        samBox->setChecked(true);
        samBox->setEnabled(false);
        if (!samOutput) {
            samOutput = true;
            sl_onSamBoxClicked();
        }
    } else {
        samBox->setEnabled(true);
    }
    DnaAssemblyGUIExtensionsFactory* gui = env->getGUIExtFactory();
    if (gui!=NULL && gui->hasMainWidget()) {
        customGUI = gui->createMainWidget(this);
        int extensionMinWidth = customGUI->sizeHint().width();
        int extensionMinHeight = customGUI->sizeHint().height();
        customGUI->setMinimumWidth(extensionMinWidth);
        customGUI->setMinimumHeight(extensionMinHeight);
        verticalLayout->insertWidget(insertPos, customGUI);
        // adjust sizes
        // HACK: add 50 to min height when dialog first shown, 50 to width always (fix for Mac OS)
        // TODO: handle margins in proper way so this hack not needed
        setMinimumHeight(customGUI->minimumHeight() + minimumHeight() + macFixDelta);
        if (minimumWidth() < customGUI->minimumWidth() + 50) {
            setMinimumWidth(customGUI->minimumWidth() + 50);
        };
        if (!refSeqEdit->text().isEmpty()) {
            QString error;
            customGUI->buildIndexUrl(refSeqEdit->text(), prebuiltIndex, error);
        }
        customGUI->show();
        adjustSize();
    } else {
        adjustSize();
    }
}

void DnaAssemblyDialog::buildResultUrl(const GUrl& refUrl ) {
    QByteArray extension;
    if (samOutput) {
        extension = "sam";
    } else {
        extension = "ugenedb";
    }
    QString tmpUrl = QString(refUrl.dirPath() + "/" + refUrl.baseFileName()+ ".%1").arg(extension.constData());
    GUrl url = GUrlUtils::rollFileName(tmpUrl, DocumentUtils::getNewDocFileNameExcludesHint());
    resultFileNameEdit->setText(url.getURLString());
}

bool DnaAssemblyDialog::eventFilter( QObject * obj, QEvent * event ) {
    if (obj == shortReadsTable) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Delete) {
                sl_onRemoveShortReadsButtonClicked();
            }
            return true;
        } else {
            return false;
        }
    } else {
        // pass the event on to the parent class
        return QDialog::eventFilter(obj, event);
    }
}

void DnaAssemblyDialog::sl_onLibraryTypeChanged()
{
    int count = shortReadsTable->topLevelItemCount();
    for (int i = 0; i < count; ++i) {
        ShortReadsTableItem* item = static_cast<ShortReadsTableItem*> (shortReadsTable->topLevelItem(i));
        item->setLibraryType( libraryComboBox->currentIndex() == 0 ? LIBRARY_TYPE_SINGLE : LIBRARY_TYPE_PAIRED );
    }
}

void DnaAssemblyGUIUtils::runAssembly2ReferenceDialog(const QStringList& shortReadUrls, const QString& refSeqUrl) {
    DnaAssemblyDialog dlg(QApplication::activeWindow(), shortReadUrls, refSeqUrl);
    if (dlg.exec()) {
        DnaAssemblyToRefTaskSettings s;
        s.samOutput = dlg.isSamOutput();
        s.refSeqUrl = dlg.getRefSeqUrl();
        s.algName = dlg.getAlgorithmName();
        s.resultFileName = dlg.getResultFileName();
        s.setCustomSettings( dlg.getCustomSettings() );
        s.shortReadSets = dlg.getShortReadSets();
        s.prebuiltIndex = dlg.isPrebuiltIndex();
        s.openView = true;
        Task* assemblyTask = new DnaAssemblyMultiTask(s, true);
        AppContext::getTaskScheduler()->registerTopLevelTask(assemblyTask);
    }
}

ShortReadsTableItem::ShortReadsTableItem(QTreeWidget *treeWidget, const QString& url) : QTreeWidgetItem(treeWidget)
{
  
    mateTypeBox = new QComboBox(treeWidget);
    mateTypeBox->addItem(MATE_UPSTREAM);
    mateTypeBox->addItem(MATE_DOWNSTREAM);

    setData(0, 0, url);
    setData(1, 0, LIBRARY_TYPE_SINGLE);
}

GUrl ShortReadsTableItem::getUrl() const
{
    return data(0,0).toString();
}

ShortReadSet::LibraryType ShortReadsTableItem::getType() const
{
    return data(1,0).toString() == LIBRARY_TYPE_PAIRED ? ShortReadSet::PairedEndReads : ShortReadSet::SingleEndReads;
}

ShortReadSet::MateOrder ShortReadsTableItem::getOrder() const
{
    return mateTypeBox->currentText() == MATE_DOWNSTREAM ? ShortReadSet::DownstreamMate : ShortReadSet::UpstreamMate;
}

void ShortReadsTableItem::addItemToTable(ShortReadsTableItem *item, QTreeWidget *treeWidget)
{
    treeWidget->addTopLevelItem(item);
    treeWidget->setItemWidget(item,2,item->mateTypeBox);
}

void ShortReadsTableItem::setLibraryType( const QString& libraryType )
{
    setData(1,0, libraryType);
    if (libraryType == LIBRARY_TYPE_SINGLE) {
        mateTypeBox->setCurrentIndex(0);
        mateTypeBox->setEnabled(false);
    } else {
        mateTypeBox->setEnabled(true);
    }
}



} // U2
