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

#include <QMenu>

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>

#include "ToolsMenu.h"

namespace U2 {

const QString ToolsMenu::LINE = "--------";
const QString ToolsMenu::TOOLS = "";

const QString ToolsMenu::SANGER_MENU = "SANGER_MENU";
const QString ToolsMenu::SANGER_CONTROL = "SANGER_CONTROL";
const QString ToolsMenu::SANGER_DENOVO = "SANGER_DENOVO";
const QString ToolsMenu::SANGER_ALIGN = "SANGER_ALIGN";

const QString ToolsMenu::NGS_MENU = "NGS_MENU";
const QString ToolsMenu::NGS_CONTROL = "NGS_CONTROL";
const QString ToolsMenu::NGS_DENOVO = "NGS_DENOVO";
const QString ToolsMenu::NGS_MAP = "NGS_MAP";
const QString ToolsMenu::NGS_INDEX = "NGS_INDEX";
const QString ToolsMenu::NGS_RAW_DNA = "NGS_RAW_DNA";
const QString ToolsMenu::NGS_CALL_VARIANTS = "NGS_CALL_VARIANTS";
const QString ToolsMenu::NGS_VARIANT_EFFECT = "NGS_VARIANT_EFFECT";
const QString ToolsMenu::NGS_RAW_RNA = "NGS_RAW_RNA";
const QString ToolsMenu::NGS_RNA = "NGS_RNA";
const QString ToolsMenu::NGS_TRASCRIPT = "NGS_TRASCRIPT";
const QString ToolsMenu::NGS_RAW_CHIP = "NGS_RAW_CHIP";
const QString ToolsMenu::NGS_CHIP = "NGS_CHIP";
const QString ToolsMenu::NGS_CHIP_COVERAGE = "NGS_CHIP_COVERAGE";
const QString ToolsMenu::NGS_COVERAGE = "NGS_COVERAGE";
const QString ToolsMenu::NGS_CONSENSUS = "NGS_CONSENSUS";
const QString ToolsMenu::NGS_CONVERT_SAM = "NGS_CONVERT_SAM";

const QString ToolsMenu::BLAST_MENU = "BLAST_MENU";
const QString ToolsMenu::BLAST_NCBI = "BLAST_NCBI";
const QString ToolsMenu::BLAST_DBP = "BLAST_DBP";
const QString ToolsMenu::BLAST_SEARCHP = "BLAST_SEARCHP";
const QString ToolsMenu::BLAST_QUERYP = "BLAST_QUERYP";
const QString ToolsMenu::BLAST_DB = "BLAST_DB";
const QString ToolsMenu::BLAST_SEARCH = "BLAST_SEARCH";

const QString ToolsMenu::MALIGN_MENU = "MALIGN_MENU";
const QString ToolsMenu::MALIGN_MUSCLE = "MALIGN_MUSCLE";
const QString ToolsMenu::MALIGN_CLUSTALW = "MALIGN_CLUSTALW";
const QString ToolsMenu::MALIGN_CLUSTALO = "MALIGN_CLUSTALO";
const QString ToolsMenu::MALIGN_MAFFT = "MALIGN_MAFFT";
const QString ToolsMenu::MALIGN_TCOFFEE = "MALIGN_TCOFFEE";
const QString ToolsMenu::MALIGN_KALIGN = "MALIGN_KALIGN";

const QString ToolsMenu::CLONING_MENU = "CLONING_MENU";
const QString ToolsMenu::CLONING_FRAGMENTS = "CLONING_FRAGMENTS";
const QString ToolsMenu::CLONING_CONSTRUCT = "CLONING_CONSTRUCT";

const QString ToolsMenu::PRIMER_MENU = "PRIMER_MENU";
const QString ToolsMenu::PRIMER_LIBRARY = "PRIMER_LIBRARY";

const QString ToolsMenu::TFBS_MENU = "TFBS_MENU";
const QString ToolsMenu::TFBS_SITECON = "TFBS_SITECON";
const QString ToolsMenu::TFBS_WEIGHT = "TFBS_WEIGHT";
const QString ToolsMenu::TFBS_SEACRH = "TFBS_SEACRH";

const QString ToolsMenu::HMMER_MENU = "HMMER_MENU";
const QString ToolsMenu::HMMER_BUILD3 = "HMMER_BUILD3";
const QString ToolsMenu::HMMER_SEARCH3 = "HMMER_SEARCH3";
const QString ToolsMenu::HMMER_SEARCH3P = "HMMER_SEARCH3P";
const QString ToolsMenu::HMMER_BUILD2 = "HMMER_BUILD2";
const QString ToolsMenu::HMMER_CALIBRATE2 = "HMMER_CALIBRATE2";
const QString ToolsMenu::HMMER_SEARCH2 = "HMMER_SEARCH2";

const QString ToolsMenu::DOTPLOT = "DOTPLOT";
const QString ToolsMenu::GENERATE_SEQUENCE = "GENERATE_SEQUENCE";
const QString ToolsMenu::EXPERT_DISCOVERY = "EXPERT_DISCOVERY";
const QString ToolsMenu::QUERY_DESIGNER = "QUERY_DESIGNER";
const QString ToolsMenu::WORKFLOW_DESIGNER = "WORKFLOW_DESIGNER";

QMap<QString, QString> ToolsMenu::actionText;
QMap<QString, QString> ToolsMenu::actionIcon;
QMap<QString, QStringList> ToolsMenu::subMenuAction;

void ToolsMenu::init() {
    actionText[SANGER_MENU] = tr("Sanger data analysis");
    actionIcon[SANGER_MENU] = ":core/images/align.png";
    subMenuAction[TOOLS] << SANGER_MENU;
    {
        subMenuAction[SANGER_MENU] << SANGER_CONTROL;
        subMenuAction[SANGER_MENU] << LINE;
        subMenuAction[SANGER_MENU] << SANGER_DENOVO;
        subMenuAction[SANGER_MENU] << SANGER_ALIGN;
    }

    actionText[NGS_MENU] = tr("NGS data analysis");
    actionIcon[NGS_MENU] = ":core/images/align.png";
    subMenuAction[TOOLS] << NGS_MENU;
    {
        subMenuAction[NGS_MENU] << NGS_CONTROL;
        subMenuAction[NGS_MENU] << LINE;
        subMenuAction[NGS_MENU] << NGS_DENOVO;
        subMenuAction[NGS_MENU] << NGS_MAP;
        subMenuAction[NGS_MENU] << NGS_INDEX;
        subMenuAction[NGS_MENU] << LINE;
        subMenuAction[NGS_MENU] << NGS_RAW_DNA;
        subMenuAction[NGS_MENU] << NGS_CALL_VARIANTS;
        subMenuAction[NGS_MENU] << NGS_VARIANT_EFFECT;
        subMenuAction[NGS_MENU] << LINE;
        subMenuAction[NGS_MENU] << NGS_RAW_RNA;
        subMenuAction[NGS_MENU] << NGS_RNA;
        subMenuAction[NGS_MENU] << NGS_TRASCRIPT;
        subMenuAction[NGS_MENU] << LINE;
        subMenuAction[NGS_MENU] << NGS_RAW_CHIP;
        subMenuAction[NGS_MENU] << NGS_CHIP;
        subMenuAction[NGS_MENU] << NGS_CHIP_COVERAGE;
        subMenuAction[NGS_MENU] << LINE;
        subMenuAction[NGS_MENU] << NGS_COVERAGE;
        subMenuAction[NGS_MENU] << NGS_CONSENSUS;
        subMenuAction[NGS_MENU] << NGS_CONVERT_SAM;
    }

    actionText[BLAST_MENU] = tr("BLAST");
    actionIcon[BLAST_MENU] = ":external_tool_support/images/ncbi.png";
    subMenuAction[TOOLS] << BLAST_MENU;
    {
        subMenuAction[BLAST_MENU] << BLAST_NCBI;
        subMenuAction[BLAST_MENU] << LINE;
        subMenuAction[BLAST_MENU] << BLAST_DBP;
        subMenuAction[BLAST_MENU] << BLAST_SEARCHP;
        subMenuAction[BLAST_MENU] << BLAST_QUERYP;
        subMenuAction[BLAST_MENU] << LINE;
        subMenuAction[BLAST_MENU] << BLAST_DB;
        subMenuAction[BLAST_MENU] << BLAST_SEARCH;
    }

    actionText[MALIGN_MENU] = tr("Multiple sequence alignment");
    actionIcon[MALIGN_MENU] = ":core/images/msa.png";
    subMenuAction[TOOLS] << MALIGN_MENU;
    {
        subMenuAction[MALIGN_MENU] << MALIGN_MUSCLE;
        subMenuAction[MALIGN_MENU] << MALIGN_CLUSTALW;
        subMenuAction[MALIGN_MENU] << MALIGN_CLUSTALO;
        subMenuAction[MALIGN_MENU] << MALIGN_MAFFT;
        subMenuAction[MALIGN_MENU] << MALIGN_TCOFFEE;
        subMenuAction[MALIGN_MENU] << MALIGN_KALIGN;
    }

    actionText[CLONING_MENU] = tr("Cloning");
    actionIcon[CLONING_MENU] = ":core/images/dna_helix.png";
    subMenuAction[TOOLS] << CLONING_MENU;
    {
        subMenuAction[CLONING_MENU] << CLONING_FRAGMENTS;
        subMenuAction[CLONING_MENU] << CLONING_CONSTRUCT;
    }

    actionText[PRIMER_MENU] = tr("Primer");
    subMenuAction[TOOLS] << PRIMER_MENU;
    {
        subMenuAction[PRIMER_MENU] << PRIMER_LIBRARY;
    }

    actionText[TFBS_MENU] = tr("Search for TFBS");
    subMenuAction[TOOLS] << TFBS_MENU;
    {
        subMenuAction[TFBS_MENU] << TFBS_SITECON;
        subMenuAction[TFBS_MENU] << TFBS_WEIGHT;
        subMenuAction[TFBS_MENU] << LINE;
        subMenuAction[TFBS_MENU] << TFBS_SEACRH;
    }

    actionText[HMMER_MENU] = tr("HMMER tools");
    actionIcon[HMMER_MENU] = ":/hmm2/images/hmmer_16.png";
    subMenuAction[TOOLS] << HMMER_MENU;
    {
        subMenuAction[HMMER_MENU] << HMMER_BUILD3;
        subMenuAction[HMMER_MENU] << HMMER_SEARCH3;
        subMenuAction[HMMER_MENU] << HMMER_SEARCH3P;
        subMenuAction[HMMER_MENU] << LINE;
        subMenuAction[HMMER_MENU] << HMMER_BUILD2;
        subMenuAction[HMMER_MENU] << HMMER_CALIBRATE2;
        subMenuAction[HMMER_MENU] << HMMER_SEARCH2;
    }

    subMenuAction[TOOLS] << DOTPLOT;
    subMenuAction[TOOLS] << GENERATE_SEQUENCE;
    subMenuAction[TOOLS] << LINE;
    subMenuAction[TOOLS] << EXPERT_DISCOVERY;
    subMenuAction[TOOLS] << QUERY_DESIGNER;
    subMenuAction[TOOLS] << WORKFLOW_DESIGNER;
}

QMenu * ToolsMenu::getToolsMenu() {
    return AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS);
}

QMenu * ToolsMenu::getMenu(const QString &menuName) {
    QMenu *tools = getToolsMenu();
    CHECK(NULL != tools, NULL);

    if (TOOLS == menuName) {
        return tools;
    }

    QMenu *subMenu = tools->findChild<QMenu*>(menuName);
    if (NULL == subMenu) {
        SAFE_POINT(actionText.contains(menuName), "Unknown tool sub menu " + menuName, NULL);
        subMenu = tools->addMenu(actionText[menuName]);
        if (actionIcon.contains(menuName)) {
            subMenu->setIcon(QIcon(actionIcon[menuName]));
        }
        subMenu->setObjectName(menuName);
    }
    return subMenu;
}

void ToolsMenu::addAction(const QString &menuName, QAction *action) {
    QMenu *menu = getMenu(menuName);
    SAFE_POINT(NULL != menu, "Can not find menu " + menuName, );

    menu->addAction(action);
}

} // U2
