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

#include <U2Algorithm/PhyTreeGeneratorRegistry.h>
#include <U2Algorithm/SubstMatrixRegistry.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/PluginModel.h>
#include <U2Core/Settings.h>
#include <U2Core/TmpDirChecker.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Core/QObjectScopedPointer.h>
#include <U2Gui/U2FileDialog.h>

#include <U2View/LicenseDialog.h>

#include "CreatePhyTreeDialogController.h"
#include "CreatePhyTreeWidget.h"
#include "ui/ui_CreatePhyTreeDialog.h"

namespace U2 {

CreatePhyTreeDialogController::CreatePhyTreeDialogController(QWidget* parent, const MAlignmentObject* mobj, CreatePhyTreeSettings& _settings) :
    QDialog(parent),
    msa(mobj->getMAlignment()),
    settings(_settings),
    settingsWidget(NULL),
    ui(new Ui_CreatePhyTree)
{
    ui->setupUi(this);

    QMap<QString, QString> helpPagesMap;
    helpPagesMap.insert("PHYLIP Neighbor Joining","16126716");
    helpPagesMap.insert("MrBayes","16126717");
    helpPagesMap.insert("PhyML Maximum Likelihood","16126715");
    new ComboboxDependentHelpButton(this, ui->buttonBox, ui->algorithmBox, helpPagesMap);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Build"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    PhyTreeGeneratorRegistry *registry = AppContext::getPhyTreeGeneratorRegistry();
    ui->algorithmBox->addItems(registry->getNameList());

    connect(ui->browseButton, SIGNAL(clicked()), SLOT(sl_browseClicked()));
    connect(ui->algorithmBox, SIGNAL(currentIndexChanged(int)), SLOT(sl_comboIndexChaged(int)));
    connect(ui->storeSettings, SIGNAL(clicked()), SLOT(sl_onStoreSettings()));
    connect(ui->restoreSettings, SIGNAL(clicked()), SLOT(sl_onRestoreDefault()));

    QString url = GUrlUtils::getNewLocalUrlByExtention(mobj->getDocument()->getURLString(), mobj->getGObjectName(), ".nwk", "");
    ui->fileNameEdit->setText(url);

    const QString defaultAlgorithm = "PHYLIP Neighbor Joining";
    ui->algorithmBox->setCurrentIndex(ui->algorithmBox->findText(defaultAlgorithm));
}

void CreatePhyTreeDialogController::accept() {
    settings.algorithmId = ui->algorithmBox->currentText();

    CHECK(checkLicense(), );
    CHECK(checkFileName(), );
    settingsWidget->fillSettings(settings);
    CHECK(checkSettings(), );
    CHECK(checkMemory(), );

    QDialog::accept();
}

void CreatePhyTreeDialogController::sl_browseClicked() {
    GUrl oldUrl = ui->fileNameEdit->text();
    QString path;
    LastUsedDirHelper lod;
    if (oldUrl.isEmpty()) {
        path = lod.dir;
    } else {
        path = oldUrl.getURLString();
    }
    GUrl newUrl = U2FileDialog::getSaveFileName(this, tr("Choose file name"), path, "Newick format (*.nwk)");

    if (newUrl.isEmpty()) {
        return;
    }
    ui->fileNameEdit->setText(newUrl.getURLString());
    lod.url = newUrl.getURLString();
}

void CreatePhyTreeDialogController::sl_comboIndexChaged(int) {
    delete settingsWidget;
    settingsWidget = NULL;
    PhyTreeGeneratorRegistry *registry = AppContext::getPhyTreeGeneratorRegistry();
    PhyTreeGenerator *generator = registry->getGenerator(ui->algorithmBox->currentText());
    SAFE_POINT(NULL != generator, "PhyTree Generator is NULL", );
    settingsWidget = generator->createPhyTreeSettingsWidget(msa, this);
    SAFE_POINT(NULL != settingsWidget, "Settings widget is NULL", );
    ui->settingsContainerLayout->addWidget(settingsWidget);
}

CreatePhyTreeDialogController::~CreatePhyTreeDialogController() {
    delete ui;
}

void CreatePhyTreeDialogController::sl_onStoreSettings() {
    settingsWidget->storeSettings();
}
void CreatePhyTreeDialogController::sl_onRestoreDefault(){
    settingsWidget->restoreDefault();
}

bool CreatePhyTreeDialogController::checkLicense() {
    if (settings.algorithmId == "PHYLIP Neighbor Joining") { // This is a bad hack :(
        QList<Plugin *> plugins = AppContext::getPluginSupport()->getPlugins();
        foreach (Plugin *plugin, plugins) {
            if (plugin->getName() == "PHYLIP") {
                if (!plugin->isLicenseAccepted()) {
                    QObjectScopedPointer<LicenseDialog> licenseDialog = new LicenseDialog(plugin);
                    const int ret = licenseDialog->exec();
                    CHECK(!licenseDialog.isNull(), false);
                    return (ret == QDialog::Accepted);
                }
            }
        }
    }

    return true;
}

bool CreatePhyTreeDialogController::checkFileName() {
    const QString fileName = ui->fileNameEdit->text();
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please, input the file name."));
        ui->fileNameEdit->setFocus();
        return false;
    }
    settings.fileUrl = fileName;

    U2OpStatus2Log os;
    GUrlUtils::validateLocalFileUrl(GUrl(fileName), os);
    if (os.hasError()) {
        QMessageBox::warning(this, tr("Error"), tr("Please, change the output file.") + "\n" + os.getError());
        ui->fileNameEdit->setFocus(Qt::MouseFocusReason);
        return false;
    }

    return true;
}

bool CreatePhyTreeDialogController::checkSettings() {
    QString msg;
    if (!settingsWidget->checkSettings(msg, settings)) {
        if (!msg.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), msg);
        }
        return false;
    }
    return true;
}

bool CreatePhyTreeDialogController::checkMemory() {
    SAFE_POINT(NULL != settingsWidget, "Settings widget is NULL", false);

    QString msg;
    const bool memCheckOk = settingsWidget->checkMemoryEstimation(msg, msa, settings);

    if (!memCheckOk) {
        QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Warning, tr("Warning"), msg, QMessageBox::Ok|QMessageBox::Cancel, this);
        mb->exec();
        CHECK(!mb.isNull(), false);

        return (mb->result() == QMessageBox::Ok);
    }

    return true;
}

}   // namespace U2
