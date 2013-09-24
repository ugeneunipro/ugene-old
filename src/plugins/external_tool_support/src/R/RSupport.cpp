/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "RSupport.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/ScriptingToolRegistry.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QMainWindow>


namespace U2 {

RSupport::RSupport(const QString& name, const QString& path) : ExternalTool(name, path) {
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/R.png");
        grayIcon = QIcon(":external_tool_support/images/R_gray.png");
        warnIcon = QIcon(":external_tool_support/images/R_warn.png");
    }

#ifdef Q_OS_WIN
    executableFileName = "Rscript.exe";
#else
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName = "Rscript";
#endif
#endif
    validMessage = "R ";
    validationArguments << "--version";

    description+=tr("Rscript interpreter");
    versionRegExp = QRegExp("(\\d+.\\d+.\\d+)");
    toolKitName = "R";

    muted = true;

    connect(this, SIGNAL(si_toolValidationStatusChanged(bool)), SLOT(sl_toolValidationStatusChanged(bool)));
}

void RSupport::sl_toolValidationStatusChanged(bool isValid) {
    Q_UNUSED(isValid);
    ScriptingTool::onPathChanged(this);
}

RModuleSupport::RModuleSupport(const QString& name, const QString& path) : ExternalToolModule(name, path) {
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/R.png");
        grayIcon = QIcon(":external_tool_support/images/R_gray.png");
        warnIcon = QIcon(":external_tool_support/images/R_warn.png");
    }

#ifdef Q_OS_WIN
    executableFileName = "Rscript.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName = "Rscript";
    #endif
#endif

    validMessage = "[1] TRUE";

    toolKitName = "R";
    dependencies << ET_R;

    muted = true;
}

RModuleGostatsSupport::RModuleGostatsSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_GOSTATS + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_GOSTATS);
}

RModuleGodbSupport::RModuleGodbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_GO_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_GO_DB);
}

RModuleHgu133adbSupport::RModuleHgu133adbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_HGU133A_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_HGU133A_DB);
}

RModuleHgu133bdbSupport::RModuleHgu133bdbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_HGU133B_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_HGU133B_DB);
}

RModuleHgu133plus2dbSupport::RModuleHgu133plus2dbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_HGU1333PLUS2_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_HGU1333PLUS2_DB);
}

RModuleHgu95av2dbSupport::RModuleHgu95av2dbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_HGU95AV2_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_HGU95AV2_DB);
}

RModuleMouse430a2dbSupport::RModuleMouse430a2dbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_MOUSE430A2_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_MOUSE430A2_DB);
}

RModuleCelegansdbSupport::RModuleCelegansdbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_CELEGANS_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_CELEGANS_DB);
}

RModuleDrosophila2dbSupport::RModuleDrosophila2dbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_DROSOPHILA2_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_DROSOPHILA2_DB);
}

RModuleOrghsegdbSupport::RModuleOrghsegdbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_ORG_HS_EG_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_ORG_HS_EG_DB);
}

RModuleOrgmmegdbSupport::RModuleOrgmmegdbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_ORG_MM_EG_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_ORG_MM_EG_DB);
}

RModuleOrgceegdbSupport::RModuleOrgceegdbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_ORG_CE_EG_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_ORG_CE_EG_DB);
}

RModuleOrgdmegdbSupport::RModuleOrgdmegdbSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_ORG_DM_EG_DB + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_ORG_DM_EG_DB);
}

RModuleSeqlogoSupport::RModuleSeqlogoSupport(const QString &name, const QString &path) : RModuleSupport(name, path) {
    description += ET_R_SEQLOGO + tr(": Rscript module for the cistrome pipeline");
    validationArguments << "-e";
    validationArguments << QString("print(require(%1))").arg(ET_R_SEQLOGO);
}

}   // namespace
