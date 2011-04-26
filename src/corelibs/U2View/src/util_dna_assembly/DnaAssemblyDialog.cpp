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

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QKeyEvent>

#include <U2Algorithm/DnaAssemblyAlgRegistry.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Misc/DialogUtils.h>

#include "DnaAssemblyDialog.h"
#include "DnaAssemblyGUIExtension.h"

namespace U2 {

QList<QString> DnaAssemblyDialog::shortReads;
QString DnaAssemblyDialog::genomePath;
QString DnaAssemblyDialog::methodName;
bool DnaAssemblyDialog::prebuiltIndex = false;

DnaAssemblyDialog::DnaAssemblyDialog(const DnaAssemblyAlgRegistry* registry, QWidget* p /* = NULL*/ )
: QDialog(p), assemblyRegistry(registry), customGUI(NULL)
{
    setupUi(this);
    QStringList names = registry->getRegisteredAlgorithmIds();
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
    shortReadsList->installEventFilter(this);
    prebuiltIndexCheckBox->setChecked(prebuiltIndex);
    sl_onAlgorithmChanged(methodNamesBox->currentText());
    sl_onPrebuiltIndexBoxClicked();
    connect(addShortreadsButton, SIGNAL(clicked()), SLOT(sl_onAddShortReadsButtonClicked()) );
    connect(removeShortReadsButton, SIGNAL(clicked()), SLOT(sl_onRemoveShortReadsButtonClicked()));
    connect(setResultFileNameButton, SIGNAL(clicked()), SLOT(sl_onSetResultFileNameButtonClicked()));
    connect(addRefButton, SIGNAL(clicked()), SLOT(sl_onAddRefButtonClicked()) );
    connect(methodNamesBox, SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_onAlgorithmChanged(const QString &)));
    connect(prebuiltIndexCheckBox, SIGNAL(clicked()), SLOT(sl_onPrebuiltIndexBoxClicked()));
    
    if (!genomePath.isEmpty()) {
        refSeqEdit->setText(genomePath);
        buildResultUrl(genomePath);
        if (NULL != customGUI) {
            QString error;
            customGUI->buildIndexUrl(genomePath, prebuiltIndex, error);
        }
    }
    foreach(const QString& read, shortReads) {
        shortReadsList->addItem(read);
    }

}

void DnaAssemblyDialog::updateState() {
    addGuiExtension();
}

void DnaAssemblyDialog::sl_onAddShortReadsButtonClicked() {
    LastOpenDirHelper lod;
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add short reads"), lod.dir);
    if (fileNames.isEmpty()) {
        return;
    }
    lod.url = fileNames.at(fileNames.count() - 1);
    foreach(const QString& f, fileNames) {
        shortReadsList->addItem(f);    
    }
}

void DnaAssemblyDialog::sl_onAddRefButtonClicked() {
    LastOpenDirHelper lod;
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
            QMessageBox::information(this, tr("DNA Assembly"), error);
            return;
        }
        if (!customGUI->isIndexOk(error, refSeqEdit->text())) {
            QMessageBox::StandardButton res = QMessageBox::warning(this, tr("DNA Assembly"), error, QMessageBox::Ok | QMessageBox::Cancel);
            if (QMessageBox::Cancel == res) {
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
    } else if (shortReadsList->count() == 0 ) {
        QMessageBox::information(this, tr("DNA Assembly"),
            tr("Short reads list is empty!") );
    } else {
        
        genomePath.clear();
        shortReads.clear();

        genomePath = refSeqEdit->text();
        int numItems = shortReadsList->count();
        for( int i =0; i < numItems; ++i) {
            shortReads.append(shortReadsList->item(i)->text());
        }
        
        QDialog::accept();
    }
}

const GUrl DnaAssemblyDialog::getRefSeqUrl() {
    return refSeqEdit->text();
}

const QList<GUrl> DnaAssemblyDialog::getShortReadUrls() {
    QList<GUrl> urls;
    int numItems = shortReadsList->count();
    for( int i =0; i < numItems; ++i) {
        urls.append(shortReadsList->item(i)->text());
    }
    return urls;
}

const QString DnaAssemblyDialog::getAlgorithmName() {
    return methodNamesBox->currentText();
}

void DnaAssemblyDialog::sl_onRemoveShortReadsButtonClicked() {
    int currentRow = shortReadsList->currentRow();
    shortReadsList->takeItem(currentRow);
    
}

void DnaAssemblyDialog::sl_onSetResultFileNameButtonClicked() {
    LastOpenDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Set result alignment file name"), lod.dir);
    if (!lod.url.isEmpty()) {
        GUrl result = lod.url;
        if (result.lastFileSuffix().isEmpty()) {
            result = QString( "%1.srfa" ).arg( result.getURLString() );
        }
        resultFileNameEdit->setText(result.getURLString());
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

const QString DnaAssemblyDialog::getResultFileName() {
    return resultFileNameEdit->text();
}

const bool DnaAssemblyDialog::isPrebuiltIndex() {
    return prebuiltIndexCheckBox->isChecked();
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
    
    // cleanup previous extension
    if (customGUI != NULL) {
        layout()->removeWidget(customGUI);         
        setMinimumHeight(minimumHeight() - customGUI->minimumHeight());
        delete customGUI;
        customGUI = NULL;
    }
    
    // insert new extension widget
    DnaAssemblyAlgorithmEnv* env = assemblyRegistry->getAlgorithm(methodNamesBox->currentText());
    if (NULL == env) {
        adjustSize();
        return;
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
        setMinimumHeight(customGUI->minimumHeight() + minimumHeight());
        if (minimumWidth() < customGUI->minimumWidth()) {
            setMinimumWidth(customGUI->minimumWidth());
        };
        if (!refSeqEdit->text().isEmpty()) {
            QString error;
            customGUI->buildIndexUrl(refSeqEdit->text(), prebuiltIndex, error);
        }
        customGUI->show();
    } else {
        adjustSize();
    }
}

void DnaAssemblyDialog::buildResultUrl(const GUrl& refUrl ) {
    GUrl url = GUrlUtils::rollFileName(refUrl.dirPath() + "/" + refUrl.baseFileName()+ ".sam", DocumentUtils::getNewDocFileNameExcludesHint());
    resultFileNameEdit->setText(url.getURLString());
}

bool DnaAssemblyDialog::eventFilter( QObject * obj, QEvent * event ) {
    if (obj == shortReadsList) {
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

} // U2
