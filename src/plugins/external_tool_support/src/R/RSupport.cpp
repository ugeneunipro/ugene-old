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

#include "RSupport.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"
#include "conduct_go/ConductGOSupport.h"
#include "seqpos/SeqPosSupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/ScriptingToolRegistry.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Gui/MainWindow.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMainWindow>
#else
#include <QtWidgets/QMainWindow>
#endif

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
#if defined(Q_OS_UNIX)
    executableFileName = "Rscript";
#endif
#endif
    validMessage = "R scripting front-end";
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

RModuleSupport::RModuleSupport(const QString& name) : ExternalToolModule(name) {
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/R.png");
        grayIcon = QIcon(":external_tool_support/images/R_gray.png");
        warnIcon = QIcon(":external_tool_support/images/R_warn.png");
    }

#ifdef Q_OS_WIN
    executableFileName = "Rscript.exe";
#else
    #if defined(Q_OS_UNIX)
    executableFileName = "Rscript";
    #endif
#endif

    versionRegExp = QRegExp("(\\d+.\\d+.\\d+)");

    validationArguments << "-e";

    toolKitName = "R";
    dependencies << ET_R;

    errorDescriptions.insert("character(0)", tr("R module is not installed. "
                                                "Install module or set path "
                                                "to another R scripts interpreter "
                                                "with installed module in "
                                                "the External Tools settings"));

    muted = true;
}

QString RModuleSupport::getScript() const {
    return QString("list <- installed.packages();list[grep('%1',rownames(list))];list['%1','Version'];");
}

RModuleGostatsSupport::RModuleGostatsSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_GOSTATS + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_GOSTATS);
    validMessage = QString("\"%1\"").arg(ET_R_GOSTATS);
}

RModuleGodbSupport::RModuleGodbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_GO_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_GO_DB);
    validMessage = QString("\"%1\"").arg(ET_R_GO_DB);
}

RModuleHgu133adbSupport::RModuleHgu133adbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_HGU133A_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_HGU133A_DB);
    validMessage = QString("\"%1\"").arg(ET_R_HGU133A_DB);
}

RModuleHgu133bdbSupport::RModuleHgu133bdbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_HGU133B_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_HGU133B_DB);
    validMessage = QString("\"%1\"").arg(ET_R_HGU133B_DB);
}

RModuleHgu133plus2dbSupport::RModuleHgu133plus2dbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_HGU1333PLUS2_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_HGU1333PLUS2_DB);
    validMessage = QString("\"%1\"").arg(ET_R_HGU1333PLUS2_DB);
}

RModuleHgu95av2dbSupport::RModuleHgu95av2dbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_HGU95AV2_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_HGU95AV2_DB);
    validMessage = QString("\"%1\"").arg(ET_R_HGU95AV2_DB);
}

RModuleMouse430a2dbSupport::RModuleMouse430a2dbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_MOUSE430A2_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_MOUSE430A2_DB);
    validMessage = QString("\"%1\"").arg(ET_R_MOUSE430A2_DB);
}

RModuleCelegansdbSupport::RModuleCelegansdbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_CELEGANS_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_CELEGANS_DB);
    validMessage = QString("\"%1\"").arg(ET_R_CELEGANS_DB);
}

RModuleDrosophila2dbSupport::RModuleDrosophila2dbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_DROSOPHILA2_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_DROSOPHILA2_DB);
    validMessage = QString("\"%1\"").arg(ET_R_DROSOPHILA2_DB);
}

RModuleOrghsegdbSupport::RModuleOrghsegdbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_ORG_HS_EG_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_ORG_HS_EG_DB);
    validMessage = QString("\"%1\"").arg(ET_R_ORG_HS_EG_DB);
}

RModuleOrgmmegdbSupport::RModuleOrgmmegdbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_ORG_MM_EG_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_ORG_MM_EG_DB);
    validMessage = QString("\"%1\"").arg(ET_R_ORG_MM_EG_DB);
}

RModuleOrgceegdbSupport::RModuleOrgceegdbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_ORG_CE_EG_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_ORG_CE_EG_DB);
    validMessage = QString("\"%1\"").arg(ET_R_ORG_CE_EG_DB);
}

RModuleOrgdmegdbSupport::RModuleOrgdmegdbSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_ORG_DM_EG_DB + tr(": Rscript module for the %1 tool").arg(ET_GO_ANALYSIS);
    validationArguments << getScript().arg(ET_R_ORG_DM_EG_DB);
    validMessage = QString("\"%1\"").arg(ET_R_ORG_DM_EG_DB);
}

RModuleSeqlogoSupport::RModuleSeqlogoSupport(const QString &name) : RModuleSupport(name) {
    description += ET_R_SEQLOGO + tr(": Rscript module for the %1 tool").arg(ET_SEQPOS);
    validationArguments << getScript().arg(ET_R_SEQLOGO);
    validMessage = QString("\"%1\"").arg(ET_R_SEQLOGO);
}

}   // namespace
