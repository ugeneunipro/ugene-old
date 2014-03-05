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

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QMessageBox>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Gui/AppSettingsGUI.h>
#include "Bowtie2SettingsWidget.h"
#include "Bowtie2Task.h"
#include "Bowtie2Support.h"



namespace U2 {

// Bowtie2SettingsWidget

Bowtie2SettingsWidget::Bowtie2SettingsWidget(QWidget *parent):
    DnaAssemblyAlgorithmMainWidget(parent)
{
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);

    threadsSpinBox->setMaximum(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
    threadsSpinBox->setValue(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
#ifdef Q_OS_WIN
    threadsSpinBox->setValue(1);
    threadsLabel->setVisible(false);
    threadsSpinBox->setVisible(false);
#endif
}

QMap<QString,QVariant> Bowtie2SettingsWidget::getDnaAssemblyCustomSettings() {
    QMap<QString, QVariant> settings;

    switch(modeComboBox->currentIndex()) {
    case 0:
        settings.insert(Bowtie2Task::OPTION_MODE, "--end-to-end");
        break;
    case 1:
        settings.insert(Bowtie2Task::OPTION_MODE, "--local");
    }

    settings.insert(Bowtie2Task::OPTION_MISMATCHES, mismatchesSpinBox->value());

    if(seedlenCheckBox->isChecked()) {
        settings.insert(Bowtie2Task::OPTION_SEED_LEN, seedlenSpinBox->value());
    }

    if (dpadCheckBox->isChecked()) {
        settings.insert(Bowtie2Task::OPTION_DPAD, dpadSpinBox->value());
    }

    if (gbarCheckBox->isChecked()) {
        settings.insert(Bowtie2Task::OPTION_GBAR, gbarSpinBox->value());
    }

    if(seedCheckBox->isChecked()) {
        settings.insert(Bowtie2Task::OPTION_SEED, seedSpinBox->value());
    }

    settings.insert(Bowtie2Task::OPTION_THREADS, threadsSpinBox->value());

    settings.insert(Bowtie2Task::OPTION_NOMIXED, nomixedCheckBox->isChecked());
    settings.insert(Bowtie2Task::OPTION_NODISCORDANT, nodiscordantCheckBox->isChecked());
    settings.insert(Bowtie2Task::OPTION_NOFW, nofwCheckBox->isChecked());
    settings.insert(Bowtie2Task::OPTION_NORC, norcCheckBox->isChecked());
    settings.insert(Bowtie2Task::OPTION_NOOVERLAP, nooverlapCheckBox->isChecked());
    settings.insert(Bowtie2Task::OPTION_NOCONTAIN, nocontainCheckBox->isChecked());

    return settings;
}

bool Bowtie2SettingsWidget::isParametersOk(QString& /*error*/) {

    ExternalTool* bowtie2 = AppContext::getExternalToolRegistry()->getByName(ET_BOWTIE2_ALIGN);
    ExternalTool* bowtie2Build = AppContext::getExternalToolRegistry()->getByName(ET_BOWTIE2_BUILD);
    bool bowtie2ToolsExist = bowtie2 && bowtie2Build;

    if( !bowtie2ToolsExist || bowtie2->getPath().isEmpty() || bowtie2Build->getPath().isEmpty() ) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("DNA Assembly"));
        msgBox.setInformativeText(tr("Do you want to select it now?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setText(tr("Path for <i>Bowtie2</i> and <i>Bowtie2-build</i> tools is not set."));
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes) {
            AppContext::getAppSettingsGUI()->showSettingsDialog(APP_SETTINGS_EXTERNAL_TOOLS);
        }
        return false;
    }

    return true;
}

// Bowtie2GUIExtensionsFactory

DnaAssemblyAlgorithmMainWidget *Bowtie2GUIExtensionsFactory::createMainWidget(QWidget *parent) {
    return new Bowtie2SettingsWidget(parent);
}

DnaAssemblyAlgorithmBuildIndexWidget *Bowtie2GUIExtensionsFactory::createBuildIndexWidget(QWidget *parent) {
    return new Bowtie2BuildSettingsWidget(parent);
}

bool Bowtie2GUIExtensionsFactory::hasMainWidget() {
    return true;
}

bool Bowtie2GUIExtensionsFactory::hasBuildIndexWidget() {
    return true;
}

} //namespace
