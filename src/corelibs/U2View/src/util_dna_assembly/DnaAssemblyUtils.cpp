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

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/MainWindow.h>
#include <U2Algorithm/DnaAssemblyAlgRegistry.h>
#include <U2Algorithm/DnaAssemblyMultiTask.h>
#include <U2Formats/ConvertAssemblyToSamTask.h>
#include <U2Gui/OpenViewTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/MultiTask.h>

#include "DnaAssemblyUtils.h"
#include "DnaAssemblyDialog.h"
#include "BuildIndexDialog.h"
#include "ConvertAssemblyToSamDialog.h"


namespace U2 {

DnaAssemblySupport::DnaAssemblySupport()
{
    QMenu *toolsMenu = AppContext::getMainWindow()->getTopLevelMenu( MWMENU_TOOLS );
    QMenu *dnaAssemblySub = toolsMenu->addMenu(QIcon( ":core/images/align.png" ), tr("DNA assembly"));
    dnaAssemblySub->setObjectName(MWMENU_TOOLS_ASSEMBLY);
    dnaAssemblySub->menuAction()->setObjectName("DNA assembly");

    QAction* convertAssemblyToSamAction = new QAction( tr("Convert UGENE Assembly data base to SAM format..."), this );
    convertAssemblyToSamAction->setObjectName("Convert UGENE Assembly data base to SAM format");
    convertAssemblyToSamAction->setIcon(QIcon(":core/images/align.png"));
    connect( convertAssemblyToSamAction, SIGNAL( triggered() ), SLOT( sl_showConvertToSamDialog() ) );
    dnaAssemblySub->addAction( convertAssemblyToSamAction );

    QMenu *refAlignSub = toolsMenu->addMenu(QIcon( ":core/images/align.png" ), tr("Align to reference"));
    refAlignSub->menuAction()->setObjectName("Align to reference");
    refAlignSub->setObjectName(MWMENU_TOOLS_REF_ALIGN);

    QAction* dnaAssemblyAction = new QAction( tr("Align short reads"), this );
    dnaAssemblyAction->setObjectName("Align short reads");
    dnaAssemblyAction->setIcon(QIcon(":core/images/align.png"));
    connect( dnaAssemblyAction, SIGNAL( triggered() ), SLOT( sl_showDnaAssemblyDialog() ) );
    refAlignSub->addAction( dnaAssemblyAction );

    QAction* buildIndexAction = new QAction( tr("Build index"), this );
    buildIndexAction->setObjectName("Build index");
    buildIndexAction->setIcon(QIcon(":core/images/align.png"));
    connect( buildIndexAction, SIGNAL( triggered() ), SLOT( sl_showBuildIndexDialog() ) );
    refAlignSub->addAction( buildIndexAction );
}

void DnaAssemblySupport::sl_showDnaAssemblyDialog() 
{
    DnaAssemblyAlgRegistry* registry = AppContext::getDnaAssemblyAlgRegistry();
    if (registry->getRegisteredAlgorithmIds().isEmpty()) {
        QMessageBox::information(QApplication::activeWindow(), tr("DNA Assembly"),
            tr("There are no algorithms for DNA assembly available.\nPlease, check your plugin list.") );
        return;
    }
        
    DnaAssemblyDialog dlg(QApplication::activeWindow());
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

void DnaAssemblySupport::sl_showBuildIndexDialog()
{
    DnaAssemblyAlgRegistry* registry = AppContext::getDnaAssemblyAlgRegistry();
    if (registry->getRegisteredAlgorithmIds().isEmpty()) {
        QMessageBox::information(QApplication::activeWindow(), tr("DNA Assembly"),
            tr("There are no algorithms for DNA assembly available.\nPlease, check your plugin list.") );
        return;
    }

    BuildIndexDialog dlg(registry, QApplication::activeWindow());
    if (dlg.exec()) {
        DnaAssemblyToRefTaskSettings s;
        s.refSeqUrl = dlg.getRefSeqUrl();
        s.algName = dlg.getAlgorithmName();
        s.resultFileName = dlg.getIndexFileName();
        s.indexFileName = dlg.getIndexFileName();
        s.setCustomSettings( dlg.getCustomSettings() );
        s.openView = false;
        s.prebuiltIndex = false;
        Task* assemblyTask = new DnaAssemblyMultiTask(s, false, true);
        AppContext::getTaskScheduler()->registerTopLevelTask(assemblyTask);
    }
}

void DnaAssemblySupport::sl_showConvertToSamDialog() {
    ConvertAssemblyToSamDialog dlg(QApplication::activeWindow());
    if (dlg.exec()) {
        Task *convertTask = new ConvertAssemblyToSamTask(dlg.getDbFileUrl(), dlg.getSamFileUrl());
        AppContext::getTaskScheduler()->registerTopLevelTask(convertTask);
    }
}


} // U2

