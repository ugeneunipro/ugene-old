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

#include "MSAAlignDialog.h"
#include "MSAAlignGUIExtension.h"
#include <ui/ui_PerformAlignmentDialog.h>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>

#include <U2Algorithm/MSAAlignAlgRegistry.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/LastUsedDirHelper.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QKeyEvent>

namespace U2 {

MSAAlignDialog::MSAAlignDialog(const QString& _algorithmName, bool useFileMenu ,QWidget* p)
    : QDialog(p), algorithmName(_algorithmName), customGUI(NULL), algoEnv(NULL), openFileMode(useFileMenu)
{
    ui = new Ui_performAlignmentDialog;
    ui->setupUi(this);
    if (!useFileMenu) {
        ui->inputLabel->setVisible(false);
        ui->fileNameEdit->setVisible(false);
        ui->browseButton->setVisible(false);
    }

    addGuiExtension();        
    connect(ui->browseButton, SIGNAL(clicked()), SLOT(sl_onFileNameButtonClicked()));
}


void MSAAlignDialog::sl_onFileNameButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getOpenFileName(this, tr("Set result alignment file name"), lod.dir, 
        DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true));
    if (!lod.url.isEmpty()) {
        ui->fileNameEdit->setText(lod.url);
    }
}


const QString MSAAlignDialog::getAlgorithmName() {
    return algorithmName;
}

QMap<QString, QVariant> MSAAlignDialog::getCustomSettings() {
    if (customGUI != NULL) {
        return customGUI->getMSAAlignCustomSettings();
    } else {
        return QMap<QString, QVariant>();
    }
}

void MSAAlignDialog::addGuiExtension() {
    static const int insertPos = 1;

    // cleanup previous extension
    if (customGUI != NULL) {
        layout()->removeWidget(customGUI);         
        setMinimumHeight(minimumHeight() - customGUI->minimumHeight());
        delete customGUI;
        customGUI = NULL;
    }

    MSAAlignAlgRegistry* registry = AppContext::getMSAAlignAlgRegistry();

    // insert new extension widget
    MSAAlignAlgorithmEnv* env = registry->getAlgorithm(algorithmName);
    if (env == NULL) {
        adjustSize();
        return;
    }
    MSAAlignGUIExtensionsFactory* gui = env->getGUIExtFactory();
    if (gui!=NULL && gui->hasMainWidget()) {
        customGUI = gui->createMainWidget(this);
        int extensionMinWidth = customGUI->sizeHint().width();
        int extensionMinHeight = customGUI->sizeHint().height();
        customGUI->setMinimumWidth(extensionMinWidth);
        customGUI->setMinimumHeight(extensionMinHeight);
        ui->globalLayout->insertWidget(insertPos, customGUI);
        // adjust sizes
        setMinimumHeight(customGUI->minimumHeight() + minimumHeight());
        if (minimumWidth() < customGUI->minimumWidth()) {
            QMargins margins = layout()->contentsMargins();
            setMinimumWidth(customGUI->minimumWidth() + margins.left() + margins.right());
        };
        if (!customGUI->windowTitle().isEmpty()) {
            setWindowTitle(customGUI->windowTitle());
        }
        customGUI->show();
    } 
    adjustSize();
}

void MSAAlignDialog::accept() {    
    if ( openFileMode && ui->fileNameEdit->text().isEmpty() ) {
        QMessageBox::warning(this, windowTitle(), tr("The input file url is empty.\nPlease select file to align."));
        return;
    }
    
    QDialog::accept();
}

const QString MSAAlignDialog::getFileName()
{
    return ui->fileNameEdit->text();
}

} // U2
